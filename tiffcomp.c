// Copyright (c) 2016 Tetsuya Hori
// Released under the MIT license
// https://opensource.org/licenses/mit-license.php

/*
 *	堀哲也 2016/11/05
 *
 *	tiffcomp.c
 *
 *	TIFF形式の夜景・星景写真比較明合成バッチ処理における合成処理手続き
 *
 *	TIFFファイルが2つ与えられ、画像データ以外は全てそのまま、画像データはピクセル毎に比較明合成を行い、出力する
 *	入力ファイル1をベースとし、入力ファイル2により明るいピクセルがあれば、その部分をファイル2の内容で置き換える
 *	データのコピーは1つ目のファイルから全てコピー
 *
 *	TIFFのファイル構造については以下を参照
 *	http://symfo.web.fc2.com/blog/tiff_hex.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "progressbar.h"

// 大きいほど高速だが、環境によっては大きすぎるとセグフォ MBAでは3000が安定
#define OUTPUTFILE "output.tif"	// 出力するファイルの名前
#define BYTENUM 1 	// 1Byte = 8bitのTIFFを想定

typedef struct _tiff {
	FILE *fp;				/* ファイルポインタ */
	int imgHeight;			/* 画像の縦幅 */
	int imgWidth;			/* 画像の横幅 */
	long imgPos;			/* 画像データの開始位置 */
} TIFF;

int comp(int fileNum, char *file[]);
int readValue(TIFF *image);
int checkPixel(TIFF image[], FILE *fpw, int fileNum);
void compare(int array[], TIFF *image, int pixNum);
unsigned long culcBrightness(int *rgb);
void clearArray(int array[], int num);

int main(int argc, char *argv[]){
	int i;
	argc--;	// 総ファイル数
	
	// 処理にかかる時間を測る
	clock_t timer;
	timer = clock();
	
	if(argc < 2){		// 合成ファイルは2枚以上必要
		printf("lack of file error\n");
		exit(1);
	}
	
	// 合成
	if(comp(argc, &argv[1])){	
		printf("comp error\n");
		exit(1);
	}
	
	printf("done %fs\n", (float)(clock() - timer) / CLOCKS_PER_SEC);
	exit(0);
}

// ファイル数fileNum, ファイル名へのポインタ配列*file[]を引数とし、合成の手続きを行う
// 成功時に0、失敗時に1が返る
int comp(int fileNum, char *file[]){
	FILE *fpw;
	int pixNum;					// 総ピクセル数 = imgHeight * imgWidth
	int width, height;			// 解像度チェック用
	int i;
	long buf;						// 入出力用バッファ
	
	// 構造体配列の動的宣言
	// 0で初期化しないと出力画像が狂うので、callocを使う
	TIFF *input;
	if((input = (TIFF *)calloc(fileNum, sizeof(TIFF))) == NULL){
		printf("calloc error\n");
		return 1;
	}
	
	// ファイルの読み込み及び作成
	for(i = 0; i < fileNum; i++){
		if ((input[i].fp = fopen(file[i], "rb")) == NULL) {
			printf("No.%d file open error\n", i);
			return 1;
		}
	}
	
	// 出力ファイル
	fpw = fopen(OUTPUTFILE, "wb");
	
	/*-------------------------- データ処理スタート ---------------------------*/
	
	/* -------- 先に一旦全データをコピーしておく ------*/
	// データのコピーは入力ファイル1から行う
	while (fread(&buf, 1, 16, input[0].fp)) {		// tiffファイル内には他のアプリケーションが独自に書いたデータがある
		fwrite(&buf, 1, 16, fpw);							// これらを漏れなくコピーするために予め全データをコピーする
	}	// このステップでは入力ファイル2は不要
	
	// 読み込み
	for(i = 0; i < fileNum; i++){
		if(readValue(&input[i]))
			return 1;
	}
	
	/* ---------- 画像データ解像度同一チェック ----------- */
	height = input[0].imgHeight;
	width = input[0].imgWidth;
	for(i = 1; i < fileNum; i++){
		if(height != input[i].imgHeight || width != input[i].imgWidth){
			printf("image size error\n");
			return 1;
		}
	}
		
	if(checkPixel(input, fpw, fileNum))	// 画像処理
		return 1;
		
	for(i = 0; i < fileNum; i++)
		fclose(input[i].fp);
	
	fclose(fpw);
	free(input);
	
	printf("file closed\n");
	return 0;
}

// 各種データ読み込み
int readValue(TIFF *image){
	int i;
	int num;
	long header = 0;		// 入力データが狂わないよう念のため0で初期化
	int idfPos;
	FILE *fp = image->fp;	// 構造体のメンバに何度もアクセスしないため
	
	fseek(fp, 4, SEEK_SET);			// IDFポインタまで移動
	fread(&idfPos, 4, 1, fp);	// IDFデータ開始位置を格納
	fseek(fp, idfPos, SEEK_SET);	// IDFデータ開始位置に移動
	
	/*---------- IDFデータ -----------*/
	fread(&num, 2, 1, fp);	// エントリカウント数の確認
		
	// エントリの内容　（画像データの開始位置はファイルごとに違うため、入力ファイル2についても開始位置を調べる）
	for(i = 0; i < num; i++){
		fread(&header, 2, 1, fp); // エントリタグをチェック
		
		switch (header){
		case 0x0100:	// 画像の横幅
			// データ位置まで移動
			fseek(fp, 6, SEEK_CUR);
			fread(&image->imgWidth, 4, 1, fp);
			break;
		case 0x0101:	// 画像の縦幅
			// データ位置まで移動
			fseek(fp, 6, SEEK_CUR);
			fread(&image->imgHeight, 4, 1, fp);
			break;
		case 0x0111:	// 画像データの開始位置を格納している場合
			// データ位置まで移動
			fseek(fp, 6, SEEK_CUR);
			// 画像データの開始位置を得る
			fread(&image->imgPos, 4, 1, fp);
			break;
		default:
			fseek(fp, 10, SEEK_CUR); // 次のエントリダグまで移動
			break;
		}
	}
	/* ---------- 画像データの開始位置が存在するかチェック ----------- */
	if(image->imgPos == 0){
		printf("image position error\n");
		printf("%d, %lx ", i, image->imgPos);
		return 1;
	}
	return 0;
}

/* --------- 画像処理部分 --------- */
// image[] はTIFF型構造体配列
// fileNum個の配列を引数に取る
int checkPixel(TIFF image[], FILE *fpw, int fileNum){
	int i = 0, j;
	int temp;
	int pixNum = image[0].imgHeight * image[0].imgWidth;	// 総画素数を計算
	
	/* 比較処理のフェーズ化を行う */
	const int JOB = 3000;		// 比較明合成処理をフェーズ化して行う時、一度に処理する画素数
	const int job = JOB * 3;		// 1フェーズに処理するバイト数 = 1フェーズに処理する画素数 * 3 (RGB)
	const int JOBE = pixNum % JOB;	// 最後に追加で処理する画素数
	const int jobe = JOBE * 3;
	int array[JOB * 3] = {0};	// 色データを格納
	const int phaze = pixNum / JOB;		// 総フェーズ数
	
	/* 作業開始前に1度プログレスバーを見せておく */
	if(simpleProgress(i, phaze))	// 何フェーズ終わったか == 全体のうちiフェーズ終わった
		return 1;
	
	// 画像データ開始位置へ移動
	for(i = 0; i < fileNum; i++){
		fseek(image[i].fp, image[i].imgPos, SEEK_SET);
	}
	fseek(fpw, image[0].imgPos, SEEK_SET);
	
	/*------- フェーズ化した処理 -------*/
	for(i = 0; i < phaze; i++){
		clearArray(array, job);
		for(j = 0; j < fileNum; j++)
			compare(array, &image[j], JOB);
		
		// 配列から16バイトずつ書き込み
		for(j = 0; j < job; j++)
			fwrite(&array[j], BYTENUM, 1, fpw);
		
		// 途中経過の表示
		if(simpleProgress(i, phaze) && (i%5) == 0)		// 表示は5回に1回くらいでいい
			return 1;
	}
	printf("normal phaze end\n");
	/*-------- 残った領域を追加処理 --------*/
	clearArray(array, job);
	for(j = 0; j < fileNum; j++)
		compare(array, &image[j], JOBE);
	
	for(j = 0; j < jobe; j++)		// 配列から1バイトずつ書き込み
		fwrite(&array[j], BYTENUM, 1, fpw);
	/*------------------------------------------*/
	if(simpleProgress(i, phaze))	// この時i == phazeなので100%が表示される
		return 1;
	printf("additional phaze end\n");
	
	return 0;
}

// numは画素数
// array[]の要素とimageのデータで比較明合成を行い、結果をarray[]に入れる
void compare(int array[], TIFF *image, int pixNum){
	int i, j;
	int pixel[3] = {0};
	FILE *fp = image->fp;	// 構造体のメンバに何度もアクセスしないため
	
	for(i = 0; i < pixNum; i++){
		for(j = 0; j < 3; j++)	// ピクセルのRGBの各数値を取得
			fread(&pixel[j], BYTENUM, 1, fp);
			
		// 2つの入力ファイルの輝度をそれぞれ計算して比較
		if(culcBrightness(&array[i * 3]) < culcBrightness(pixel)){
			for(j = 0; j < 3; j++)
				array[i * 3 + j] = pixel[j];
		}
	}
}

// 輝度計算を行う
// pchansblog.exblog.jp/26051068/ 
unsigned long culcBrightness(int *rgb){
	return 306*(unsigned long)rgb[0]+601*(unsigned long)rgb[1]+117*(unsigned long)rgb[2]+512;
}

// 配列のクリア
void clearArray(int array[], int num){
	int i = 0;
	while(i < num)
		array[i++] = 0;
}