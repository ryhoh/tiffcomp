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

typedef struct _tiff {
	FILE *fp;				/* ファイルポインタ */
	int imgHeight;			/* 画像の縦幅 */
	int imgWidth;			/* 画像の横幅 */
	int bitNum;             /* ビットの深さ */
	long imgPos;			/* 画像データの開始位置 */
} TIFF;

int comp(int fileNum, char *file[]);
int readValue(TIFF *image);
int checkPixel(TIFF image[], FILE *fpw, int fileNum);
int checkPixel16(TIFF image[], FILE *fpw, int fileNum);
void compare(unsigned char array[], FILE *fp, int pixNum);
void compare16(unsigned short array[], FILE *fp, int pixNum);
void clearArray(unsigned char array[], int num);
void clearArray16(unsigned short array[], int num);

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
	int height = input[0].imgHeight;
	int width = input[0].imgWidth;
	for(i = 1; i < fileNum; i++){
		if(height != input[i].imgHeight || width != input[i].imgWidth){
			printf("image size error\n");
			return 1;
		}
	}
	/* ---------- ビット深度同一チェック ----------- */
	int bitNum = input[0].bitNum;
	for(i = 1; i < fileNum; i++) {
	    if(bitNum != input[i].bitNum) {
	        printf("bitNum error, %d\n", input[i].bitNum);
	        return 1;
	    }
	}
	
	// 画像情報
	printf("%d x %d\n", input[0].imgHeight, input[0].imgWidth);
	printf("%d bit\n", input[0].bitNum);
	
	// 画像処理
	if(input[0].bitNum == 8) {
	    if(checkPixel(input, fpw, fileNum))
		    return 1;
	} else if(input[0].bitNum == 16) {
	    if(checkPixel16(input, fpw, fileNum))
		    return 1;
	} else {
	    printf("invalid bitNum\n");
	    return 1;
	}
		
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
		case 0x0102:    // ビットの深さ
		    // データ位置まで移動
			fseek(fp, 6, SEEK_CUR);
			fread(&image->bitNum, 4, 1, fp);
			if(image->bitNum > 16) {    // 有効な数値ではなく格納位置があった場合
			    long buff = ftell(fp);
			    fseek(fp, image->bitNum, SEEK_SET);
			    fread(&image->bitNum, 2, 1, fp);
			    fseek(fp, buff, SEEK_SET);
			}
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
    const int BYTENUM = 1;
    
	int i = 0, j = 0, cur;
	int temp;
	int pixNum = image[0].imgHeight * image[0].imgWidth;	// 総画素数を計算
	
	/* 比較処理のフェーズ化を行う */
	const int JOB = 10000;		// 比較明合成処理をフェーズ化して行う時、一度に処理する画素数
	const int job = JOB * 3;		// 1フェーズに処理するバイト数 = 1フェーズに処理する画素数 * 3 (RGB)
	const int JOBE = pixNum % JOB;	// 最後に追加で処理する画素数
	const int jobe = JOBE * 3;
	const int phaze = pixNum / JOB;		// 総フェーズ数
	
	unsigned char array[JOB * 3] = {0};	// 色データを格納
	
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
			compare(array, image[j].fp, JOB);
		
		// 配列から書き込み
		cur = 0;
		for(j = 0; j < job / 16; j++) {
			fwrite(&array[cur], BYTENUM, 16, fpw);
			cur += 16;
		}
		for(j = cur; j < job; j++) {
			fwrite(&array[j], BYTENUM, 1, fpw);
		}
		
		// 途中経過の表示
		if(simpleProgress(i, phaze) && (i%5) == 0)		// 表示は5回に1回くらいでいい
			return 1;
	}
	printf("normal phaze end\n");
	/*-------- 残った領域を追加処理 --------*/
	clearArray(array, jobe);
	for(j = 0; j < fileNum; j++)
		compare(array, image[j].fp, JOBE);
	
	// 配列から書き込み
	cur = 0;
	for(j = 0; j < jobe / 16; j++) {
		fwrite(&array[cur], BYTENUM, 16, fpw);
		cur += 16;
	}
	for(j = cur; j < jobe; j++) {
		fwrite(&array[j], BYTENUM, 1, fpw);
	}
	/*------------------------------------------*/
	if(simpleProgress(i, phaze))	// この時i == phazeなので100%が表示される
		return 1;
	printf("additional phaze end\n");
	
	return 0;
}

// 16bit対応版
int checkPixel16(TIFF image[], FILE *fpw, int fileNum){
    const int BYTENUM = 2;
    
	int i = 0, j = 0, cur;
	int temp;
	int pixNum = image[0].imgHeight * image[0].imgWidth;	// 総画素数を計算
	
	/* 比較処理のフェーズ化を行う */
	const int JOB = 10000;		// 比較明合成処理をフェーズ化して行う時、一度に処理する画素数
	const int job = JOB * 6;		// 1フェーズに処理するバイト数 = 1フェーズに処理する画素数 * 6 (RRGGBB)
	const int JOBE = pixNum % JOB;	// 最後に追加で処理する画素数
	const int jobe = JOBE * 6;
	const int phaze = pixNum / JOB;		// 総フェーズ数
	const int arraySize = job / BYTENUM;
	
	unsigned short array[arraySize] = {0};	// 色データを格納
	
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
		clearArray16(array, arraySize);
		for(j = 0; j < fileNum; j++) {
			compare16(array, image[j].fp, JOB);
		}
		
		// 配列から書き込み
		cur = 0;
		for(j = 0; j < arraySize / 8; j++) {
			fwrite(&array[cur], BYTENUM, 8, fpw);
			cur += 8;
		}
		for(j = cur; j < arraySize; j++) {
			fwrite(&array[j], BYTENUM, 1, fpw);
		}
		
		// 途中経過の表示
		if(simpleProgress(i, phaze) && (i%5) == 0)		// 表示は5回に1回くらいでいい
			return 1;
	}
	printf("normal phaze end\n");
	/*-------- 残った領域を追加処理 --------*/
	clearArray16(array, arraySize);
	for(j = 0; j < fileNum; j++)
		compare16(array, image[j].fp, JOBE);
	
	// 配列から書き込み
	cur = 0;
	for(j = 0; j < jobe / 2 / 8; j++) {
		fwrite(&array[cur], BYTENUM, 8, fpw);
		cur += 8;
	}
	for(j = cur; j < jobe / 2; j++) {
		fwrite(&array[j], BYTENUM, 1, fpw);
	}
	/*------------------------------------------*/
	if(simpleProgress(i, phaze))	// この時i == phazeなので100%が表示される
		return 1;
	printf("additional phaze end\n");
	
	return 0;
}

// numは画素数
// array[]の要素とimageのデータで比較明合成を行い、結果をarray[]に入れる
void compare(unsigned char array[], FILE *fp, int pixNum){
	int i, j;
	int dataSize = pixNum*3;
	unsigned char *pixels = (unsigned char *)malloc(sizeof(unsigned char) * dataSize);
	if(pixels == NULL){
		printf("malloc fault\n");
		exit(1);
	}
	
	// imageから対象データを全て読み込む
	int cur=0;
	for(i = 0; i < dataSize / 16; i++) {	// 高速化のため16byteずつ読む
		fread(&pixels[cur], 1, 16, fp);
		cur += 16;
	}
	for(i = cur; i < dataSize; i++) {
		fread(&pixels[i], 1, 1, fp);
	}
	
	// 2つの入力ファイルの輝度をそれぞれ計算して比較
	for(i = 0; i < pixNum; i++){
		if(306*array[i * 3]+601*array[i * 3 + 1]+117*array[i * 3 + 2]
		< 306*pixels[i * 3]+601*pixels[i * 3 + 1]+117*pixels[i * 3 + 2]) {
			for(j = i*3; j < i*3 + 3; j++)
				array[j] = pixels[j];
		}
	}
	
	free(pixels);
}

// 16bit対応版
void compare16(unsigned short array[], FILE *fp, int pixNum){
	int i, j;
	int arraySize = pixNum*3;
	unsigned short *pixels = (unsigned short *)malloc(sizeof(unsigned short) * pixNum*3);
	if(pixels == NULL){
		printf("malloc error\n");
		exit(1);
	}
	
	// imageから対象データを全て読み込む
	int cur=0;
	for(i = 0; i < arraySize / 8; i++) {	// 高速化のため16byteずつ読む
		fread(&pixels[cur], 2, 8, fp);
		cur += 8;
	}
	for(i = cur; i < arraySize; i++) {
		fread(&pixels[i], 2, 1, fp);
	}
	
	// 2つの入力ファイルの輝度をそれぞれ計算して比較
	for(i = 0; i < pixNum; i++){
		if(306*array[i * 3]+601*array[i * 3 + 1]+117*array[i * 3 + 2]
		< 306*pixels[i * 3]+601*pixels[i * 3 + 1]+117*pixels[i * 3 + 2]) {
			for(j = i*3; j < i*3 + 3; j++)
				array[j] = pixels[j];
		}
	}
	
	free(pixels);
}

// 配列のクリア
void clearArray(unsigned char array[], int num){
	int i = 0;
	while(i < num)
		array[i++] = 0;
}
void clearArray16(unsigned short array[], int num){
	int i = 0;
	while(i < num)
		array[i++] = 0;
}