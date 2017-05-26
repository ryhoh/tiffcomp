/*
The MIT License (MIT)
Copyright (c) 2016 tetsuya

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
 *	symfo.web.fc2.com/blog/tiff_hex.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <unistd.h>
//#include <pthread.h>
#include "progressbar.h"

#define COPYBUFFER 16	// 全データをコピーする際に、一度に何バイトをコピーするか
#define PIXELJOB 1500	// 比較明合成処理をフェーズ化して行う時、一度に処理するピクセルの数
// 大きいほど高速だが、環境によっては大きすぎるとセグフォ 開発環境(MBA)では8bit時に3000が安定
#define OUTPUTFILE "output"	// 出力するファイルの名前
//#define THREADNUM 4	// スレッド数

typedef struct _tiff {		// 画像の情報を格納する構造体配列
	FILE *fp;					/* ファイルポインタ */
	int imgHeight;			/* 画像の縦幅 */
	int imgWidth;			/* 画像の横幅 */
	long imgPos;			/* 画像データの開始位置 */
} TIFF;
/*
typedef struct _args {	// スレッドへ渡す引数をまとめた構造体
	int fileNum;
	char **file;			// ポインタ配列*argv[]の処理対象ポインタ群の先頭アドレス
	int threadNumber;
} ARGS;
*/
//void *preComp(void *threadArgs);
int comp(int fileNum, char *file[], int flag);
int readValue(TIFF *image, int *bitDepth);
int checkPixel(TIFF image[], FILE *fpw, int fileNum, int *bitDepth);
void compare(int array[], TIFF *image, int num, int *bitDepth);
void phazeProcess(TIFF image[], int array[], int fileNum, int job, int *bitDepth, FILE *fpw);
unsigned long culcBrightness(int r, int g, int b);
void clearArray(int array[], int num);

int main(int argc, char *argv[]){
	//int i;
	const int num = argc - 1;	// 総ファイル数
	//pthread_t pthread[THREADNUM];	// マルチスレッド処理用
	//int job, jobe;					// 等分した1スレッドあたりの作業ファイル数、余ったファイル数
	//ARGS *threadArgs;	// スレッドへ渡す引数
	
	// 処理にかかる時間を測る
	time_t timer;
	long startTime = time(&timer);
	
	if(num < 2){		// 合成ファイルは2枚以上必要
		printf("lack of file error\n");
		return 1;
	//} else if(num < 11){
	} else {
		if(comp(num, &argv[1], 0)){		// 10枚以下ならシングルスレッドで合成
			printf("comp error\n");
			return 1;
		}
	}
	/* else {											// マルチスレッド処理
		job = num / THREADNUM;
		jobe = num % THREADNUM;
		
		for(i = 0; i < 1; i++){
			if(i == THREADNUM - 1)
				threadArgs->fileNum = job + jobe;			// 最後のスレッドには余りのファイルも処理させる
			else
				threadArgs->fileNum = job;
				
			threadArgs->file = &argv[i * job + 1];	// ファイル群を前から等分して、処理するファイル群の先頭のポインタを渡す
			threadArgs->threadNumber = i + 1;		// スレッドに番号をつける
			
			pthread_create(&pthread[i], NULL, &preComp, (void *)threadArgs);
		}
		
		for(i = 0; i < THREADNUM; i++){
			pthread_join(pthread[i], NULL);
		}
	}*/
	
	printf("done %lds\n", time(&timer) - startTime);
	return 0;
}

// マルチスレッドで処理する場合の、compを呼び出すまでの中間手続き
/*void *preComp(void *threadArgs){
	ARGS *args = (ARGS *)threadArgs;
	
	if(comp(args->fileNum, args->file, args->threadNumber))
		exit(1);
	return 0;
}*/

// ファイル数fileNum, ファイル名へのポインタ配列*file[]を引数とし、合成の手続きを行う
// flagはマルチスレッド処理時の制御用
int comp(const int fileNum, char *file[], int flag){
	int i;
	FILE *fpw;
	int bitDepth;			// ビットの深さ
	long buf;				// 入出力用バッファ
	int readByte;			// 全データをコピーする作業を高速化するために使う
	
	char name[15] = OUTPUTFILE;	// 出力するファイル名 ここに拡張子をつける
	char *style = strstr(file[0], ".");		// 拡張子を得る
	strcat(name, style);	// 拡張子をつける
	
	//char name[3] = {0};			// 出力するファイル名（マルチスレッド処理時は可変）
	
	// 構造体配列の動的宣言
	// 0で初期化しないと画像の解像度等のデータが狂うので、callocを使う
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
	
	if(flag == 0)	// シングルスレッドでの処理
		fpw = fopen(name, "wb");
	/*
	else { // マルチスレッドでの処理
		name[0] = flag + '0';	// i to a
		fpw = fopen(name, "wb");
	}
	if(fpw == NULL){
		printf("output file making error\n");
		return 1;
	}*/
	
	/*-------------------------- データ処理スタート ---------------------------*/
	
	/* -------- 先に一旦全データをコピーしておく ------*/
	// データのコピーは入力ファイル1から行う
	while ((readByte = fread(&buf, 1, COPYBUFFER, input[0].fp)))	// tiffファイル内には他のアプリケーションが独自の形式で書いたデータがある
		fwrite(&buf, 1, readByte, fpw);													// これらを漏れなくコピーするために予め全データをコピーする
		// readByteは読み込みに成功した数（バイト）　＝　書き込む数（バイト）
	
	for(i = 0; i < fileNum; i++)
		if(readValue(&input[i], &bitDepth))		// 読み込み
			return 1;
	
	/* ---------- 画像データ解像度同一チェック ----------- */
	for(i = 1; i < fileNum; i++){
		if(input[0].imgHeight != input[i].imgHeight){
			printf("image size error\n");
			return 1;
		} else if(input[0].imgWidth != input[i].imgWidth){
			printf("image size error\n");
			return 1;
		}
	}
		
	if(checkPixel(input, fpw, fileNum, &bitDepth))	// 画像処理
		return 1;
		
	/* 終了処理 */
	for(i = 0; i < fileNum; i++)
		fclose(input[i].fp);
	
	fclose(fpw);
	free(input);
	return 0;
}

// 各種データ読み込み bitDepth:ビットの深さ
int readValue(TIFF *image, int *bitDepth){
	int i, entryNum, idfPos;
	long buf = 0, addr;	// bufはエントリ読み込み用 addrは値を間接的に（アドレスで）指定された時に、読み込み位置を退避
	// bufを0で初期化しないと、読み込んだ数値が化ける
	
	fseek(image->fp, 4, SEEK_SET);			// IDFポインタまで移動
	fread(&idfPos, 4, 1, image->fp);	// IDFデータ開始位置を格納
	fseek(image->fp, idfPos, SEEK_SET);	// IDFデータ開始位置に移動
	
	/*---------- IDFデータ（画像データの開始位置はファイルごとに違うため、全てのファイルで調べる） -----------*/
	fread(&entryNum, 2, 1, image->fp);		// エントリカウント数の確認
	
	for(i = 0; i < entryNum; i++){
		fread(&buf, 2, 1, image->fp); // エントリタグをチェック
		switch (buf){
		case 0x0100:	// 画像の横幅
			fseek(image->fp, 6, SEEK_CUR);	// データ位置まで移動
			fread(&image->imgWidth, 4, 1, image->fp);
			break;
		case 0x0101:	// 画像の縦幅
			fseek(image->fp, 6, SEEK_CUR);	// データ位置まで移動
			fread(&image->imgHeight, 4, 1, image->fp);
			break;
		case 0x0102:	// ビットの深さ
			fseek(image->fp, 6, SEEK_CUR);	// データ位置まで移動
			fread(bitDepth, 4, 1, image->fp);
			if((*bitDepth != 8) && (*bitDepth != 16)){	// 8bitでも16bitでもない = アドレスが入っている
				addr = ftell(image->fp);						// 現在の位置を退避
				fseek(image->fp, *bitDepth, SEEK_SET);	// アドレスをたどって
				fread(bitDepth, 2, 1, image->fp);					// bit情報を格納
				fseek(image->fp, addr, SEEK_SET);	// 退避した位置を戻す
			}
			*bitDepth /= 8;	// ビットではなくバイト単位で扱いたい
			break;
		case 0x0111:	// 画像データの開始位置
			fseek(image->fp, 6, SEEK_CUR);	// データ位置まで移動
			fread(&image->imgPos, 4, 1, image->fp);
			break;
		default:		// 上記以外
			fseek(image->fp, 10, SEEK_CUR); // 次のエントリダグまで移動
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
int checkPixel(TIFF image[], FILE *fpw, int fileNum, int *bitDepth){
	int i = 0;
	const int pixNum = image[0].imgHeight * image[0].imgWidth;	// 総画素数を計算
	
	/* 比較処理のフェーズ化を行う */
	int array[PIXELJOB * 3 * 2];	// 現状各色16bitを超える画像は考慮しなくてよい  
	// 1度に処理する画素数 * 3 (RGB) * 2(ビットの深さ)
	
	int job = PIXELJOB * 3 * *bitDepth;	// 1フェーズに処理するバイト数
	int extrajob = (pixNum % PIXELJOB) * 3 * *bitDepth;	// 最後に追加で処理するバイト数
	int phaze = pixNum / PIXELJOB;	// フェーズ数
	/*int *array;		// 配列の大きさがビットの深さによって変わる
	if((array = (int *)malloc(job * sizeof(int))) == NULL){
		printf("malloc error\n");
		return 1;
	}*/
	
	/* 作業開始前に1度プログレスバーを見せておく */
	if(simpleProgress(i, phaze))	// 何フェーズ終わったか == 全体のうちiフェーズ終わった
		return 1;
	
	// 画像データ開始位置へ移動
	for(i = 0; i < fileNum; i++)
		fseek(image[i].fp, image[i].imgPos, SEEK_SET);
	fseek(fpw, image[0].imgPos, SEEK_SET);
	
	// 各ファイルを横断するように処理をphaze回繰り返す
	for(i = 0; i < phaze; i++){
		phazeProcess(image, array, fileNum, job, bitDepth, fpw);
		if(i % 10 == 0)	// 途中経過の表示 回数減らすため10回に1回
			if(simpleProgress(i, phaze))
				return 1;
	}
	phazeProcess(image, array, fileNum, extrajob, bitDepth, fpw);
	if(simpleProgress(i, phaze))	// この時i == phazeなので100%が表示される
		return 1;

	return 0;
}

// フェーズ処理部分
// 引数　画像データの構造体配列, ピクセル用配列, ファイル数, 処理するデータの大きさ(Byte), ビットの深さ, fp
void phazeProcess(TIFF image[], int array[], int fileNum, int job, int *bitDepth, FILE *fpw){
	int i;
	clearArray(array, job);
	for(i = 0; i < fileNum; i++)	// ファイルを横断するように比較処理
		compare(array, &image[i], PIXELJOB, bitDepth);
	
	for(i = 0; i < job; i++)		// 配列からbitDepthバイトずつ結果を書き込む
		fwrite(&array[i], *bitDepth, 1, fpw);
}

// numは画素数
// array[]の要素とimageのデータで比較明合成を行い、結果をarray[]に入れる
void compare(int array[], TIFF *image, int num, int *bitDepth){
	int i, j;
	int pixel[3] = {0};
	
	for(i = 0; i < num; i++){
		for(j = 0; j < 3; j++)	// ピクセルのRGBの各数値を取得
			fread(&pixel[j], *bitDepth, 1, image->fp);
		
		// 2つの入力ファイルの輝度をそれぞれ計算して比較
		if(culcBrightness(array[i * 3 + 0], array[i * 3 + 1], array[i * 3 + 2]) < culcBrightness(pixel[0], pixel[1], pixel[2]))
			for(j = 0; j < 3; j++)
				array[i * 3 + j] = pixel[j];
	}
}

// 輝度計算を行う
// pchansblog.exblog.jp/26051068/ 
unsigned long culcBrightness(int r, int g, int b){
	return 306*(unsigned long)r+601*(unsigned long)g+117*(unsigned long)b+512;
}

// 配列のクリア
void clearArray(int array[], int num){
	int i;
	for(i = 0; i < num; i++)
		array[i] = 0;
}