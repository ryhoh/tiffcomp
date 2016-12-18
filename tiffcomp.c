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
#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#include "progressbar.h"

#define JOB 1000	// 比較明合成処理をフェーズ化して行う時、一度に処理するピクセルの数
#define OUTPUTFILE "output.tif"	// 出力するファイルの名前
#define BYTENUM 1 	// 1Byte = 8bitのTIFFを想定

typedef struct _tiff {
	FILE *fp;				/* ファイルポインタ */
	int idfPos;				/* IDF情報の開始位置 */
	int entryNum;			/* エントリカウント数 */
	int imgHeight;			/* 画像の縦幅 */
	int imgWidth;			/* 画像の横幅 */
	long imgPos;			/* 画像データの開始位置 */
} TIFF;

int comp(int fileNum, char *file[]);
int readValue(TIFF *image);
int checkPixel(TIFF image[], FILE *fpw, int fileNum);
void compare(int array[], TIFF *image2, int num);
unsigned long culcBrightness(int r, int g, int b);
void clearArray(int array[], int num);

// 実験的にここにmainを書く
int main(int argc, char *argv[]){
	int num = argc - 1;	// 総ファイル数
	
	// 処理速度を測る
	time_t timer;
	long speed = time(&timer);
	
	// 合成ファイルは2枚以上必要 
	if(num < 2){
		printf("lack of file error\n");
		return 1;
	}
	
	if(comp(num, &argv[1])){
		printf("comp error\n");
		return 1;
	}
	
	printf("%ld\n", time(&timer) - speed);
	return 0;
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
	// 0で初期化しないと出力画像がピンク1色になるので、callocを使う
	TIFF *input;
	if((input = (TIFF *)calloc(fileNum, sizeof(TIFF))) == NULL){
		printf("calloc error\n");
		return 1;
	}
	
	// ファイルの読み込み及び作成
	for(i = 0; i < fileNum; i++){
		if ((input[i].fp = fopen(file[i], "rb")) == NULL) {
			printf("file open error\n");
			return 1;
		}
	}
	fpw = fopen(OUTPUTFILE, "wb");
	
	/*-------------------------- データ処理スタート ---------------------------*/
	
	/* -------- 先に一旦全データをコピーしておく ------*/
	// データのコピーは入力ファイル1から行う
	while (fread(&buf, 1, 1, input[0].fp)) {		// tiffファイル内には他のアプリケーションが独自に書いたデータがある
		fwrite(&buf, 1, 1, fpw);							// これらを漏れなくコピーするために予め全データをコピーする
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
		if(height != input[i].imgHeight){
			printf("image size error\n");
			return 1;
		} else if(width != input[i].imgWidth){
			printf("image size error\n");
			return 1;
		}
	}
		
	if(checkPixel(input, fpw, fileNum))	// 画像処理
		return 1;
		
	for(i = 0; i < fileNum; i++){
		fclose(input[i].fp);
	}
	fclose(fpw);
	
	free(input);
	return 0;
}

// 各種データ読み込み
int readValue(TIFF *image){
	int i;
	long buf;
	
	fseek(image->fp, 4, SEEK_SET);			// IDFポインタまで移動
	fread(&image->idfPos, 4, 1, image->fp);	// IDFデータ開始位置を格納
	fseek(image->fp, image->idfPos, SEEK_SET);	// IDFデータ開始位置に移動
	
	/*---------- IDFデータ -----------*/
	fread(&image->entryNum, 2, 1, image->fp);	// エントリカウント数の確認
		
	// エントリの内容　（画像データの開始位置はファイルごとに違うため、入力ファイル2についても開始位置を調べる）
	buf = 0;	// buf内に数値が残っている場合があるようなので、エントリタグ前でリセットする
	for(i = 0; i < image->entryNum; i++){
		fread(&buf, 2, 1, image->fp); // エントリタグをチェック
		switch (buf){
		case 0x0100:	// 画像の横幅
			// データ位置まで移動
			fseek(image->fp, 6, SEEK_CUR);
			fread(&image->imgWidth, 4, 1, image->fp);
			break;
		case 0x0101:	// 画像の縦幅
			// データ位置まで移動
			fseek(image->fp, 6, SEEK_CUR);
			fread(&image->imgHeight, 4, 1, image->fp);
			break;
		case 0x0111:	// 画像データの開始位置を格納している場合
			// データ位置まで移動
			fseek(image->fp, 6, SEEK_CUR);
			// 画像データの開始位置を得る
			fread(&image->imgPos, 4, 1, image->fp);
			break;
		default:
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
// 成功で0, 失敗で1を返す
int checkPixel(TIFF image[], FILE *fpw, int fileNum){
	int i = 0, j, k;
	int pixNum = image[0].imgHeight * image[0].imgWidth;	// 総画素数を計算
	
	/* 比較処理のフェーズ化を行う */
	int job = JOB * 3;		// 1フェーズに処理するバイト数 = 1フェーズに処理する画素数 * 3 (RGB)
	int jobe = (pixNum % JOB) * 3;	// 最後に追加で処理するバイト数
	int array[JOB * 3] = {0};	// 要素job個
	int phaze = pixNum / JOB;	// フェーズ数
	
	/* 作業開始前に1度プログレスバーを見せておく */
	if(simpleProgress(i, phaze))	// 何フェーズ終わったか == 全体のうちiフェーズ終わった
		return 1;
	
	// 画像データ開始位置へ移動
	for(i = 0; i < fileNum; i++){
		fseek(image[i].fp, image[i].imgPos, SEEK_SET);
	}
	fseek(fpw, image[0].imgPos, SEEK_SET);
	
	//各ピクセルごとの操作
	for(i = 0; i < phaze; i++){
		clearArray(array, job);
		for(j = 0; j < fileNum; j++){
		compare(array, &image[j], JOB);
		}
		for(k = 0; k < job; k++){	// 配列から1バイトずつ書き込み
			fwrite(&array[k], BYTENUM, 1, fpw);
		}
		// 途中経過の表示
		if(simpleProgress(i, phaze))
			return 1;
	}
	
	clearArray(array, job);
	for(j = 0; j < fileNum; j++){
		compare(array, &image[j], jobe);
	}
	for(k = 0; k < jobe; k++){		// 配列から1バイトずつ書き込み
		fwrite(&array[k], BYTENUM, 1, fpw);
	}
	
	if(simpleProgress(i, phaze))	// この時i == phazeなので100%が表示される
			return 1;
	
	return 0;
}

// numは画素数
// array[]の要素とimageのデータで比較明合成を行い、結果をarray[]に入れる
void compare(int array[], TIFF *image, int num){
	int i, j;
	int pixel[3] = {0};
	
	for(i = 0; i < num; i++){
		for(j = 0; j < 3; j++){	// ピクセルのRGBの各数値を取得
			fread(&pixel[j], BYTENUM, 1, image->fp);
		}
		// 2つの入力ファイルの輝度をそれぞれ計算して比較
		if(culcBrightness(array[i * 3 + 0], array[i * 3 + 1], array[i * 3 + 2]) < culcBrightness(pixel[0], pixel[1], pixel[2])){
			for(j = 0; j < 3; j++){
				array[i * 3 + j] = pixel[j];
			}
		}
	}
}

// 輝度計算を行う
// http://pchansblog.exblog.jp/26051068/ 
unsigned long culcBrightness(int r, int g, int b){
	return 306*(unsigned long)r+601*(unsigned long)g+117*(unsigned long)b+512;
}

// 配列のクリア
void clearArray(int array[], int num){
	int i;
	for(i = 0; i < num; i++)
		array[i] = 0;
}