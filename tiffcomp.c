/*
 *	tiffcomp.c
 *
 *
 *		----tiffcomp.h----
 *		||				||
 *	main.c	 <---	<tiffcomp.c>
 *
 *	TIFF形式の夜景・星景写真比較明合成バッチ処理における合成処理手続き
 *
 *	TIFFファイルが2つ与えられ、画像データ以外は全てそのまま、画像データはピクセル毎に比較明合成を行い、出力する
 *	入力ファイル1をベースとし、入力ファイル2により明るいピクセルがあれば、その部分をファイル2の内容で置き換える
 *	データのコピーは1つ目のファイルから全てコピー
 *
 *
 *	TIFFのファイル構造については以下を参照
 *	http://symfo.web.fc2.com/blog/tiff_hex.html
 *
 *
 */
#include<stdio.h>
#include "tiffcomp.h"

unsigned long culc(int a, int b, int c){
	return 306*(unsigned long)a+601*(unsigned long)b+117*(unsigned long)c+512;
}

int comp2(char file1[], char file2[]){
	FILE *fpw;
	int pixNum, i, j;
	long buf;
	struct tiff input[2] = {0};
	
	/*
	 * buf 入出力用バッファ
	 * pixNum 総ピクセル数 = imgHeight * imgWidth
	 */
	
	// ファイルの読み込み及び作成
	if ((input[0].fp = fopen(file1, "rb")) == NULL) {
		printf("file open error\n");
		return 1;
	}
	if ((input[1].fp = fopen(file2, "rb")) == NULL) {
		printf("file open error\n");
		return 1;
	}
	fpw = fopen(OUTPUTFILE, "wb");
	
	
	/*-------------------------- データ処理スタート ---------------------------*/
	// データのコピーは入力ファイル1から行う
	
	// バイトオーダー、バージョンは共通なのでコピー
	fread(&buf, 4, 1, input[0].fp);
	fseek(input[1].fp, 4, SEEK_CUR);	// 次のステップで入力ファイル2も使うので、同じペースで進める
	fwrite(&buf, 4, 1, fpw);
	
	// IDFポインタをコピー
	for(i = 0; i < 2; i++){
		fread(&input[i].idfPos, 4, 1, input[i].fp);	// IDFデータ開始位置を格納
	} 
	fwrite(&input[0].idfPos, 4, 1, fpw);
	
	/* -------- 先に一旦全データをコピーしておく ------*/
	while (fread(&buf, 1, 1, input[0].fp)) {		// tiffファイル内には他のアプリケーションが独自に書いたデータがある
		fwrite(&buf, 1, 1, fpw);							// これらを漏れなくコピーするために予め全データをコピーする
	}	// このステップでは入力ファイル2は不要
	
	// IDFデータ開始位置に移動
	for(i = 0; i < 2; i++){
		fseek(input[i].fp, input[i].idfPos, SEEK_SET);
	}
	
	/*---------- IDFデータ -----------*/
	// エントリカウント数の確認
	for(i = 0; i < 2; i++){
		fread(&input[i].entryNum, 2, 1, input[i].fp);
	}
	
	buf = 0;	// buf内に数値が残っている場合があるようなのでリセットする
	
	// エントリの内容
	// 画像データの開始位置はファイルごとに違うため、入力ファイル2についても開始位置を調べる
	for(i = 0; i < 2; i++){
		for(j = 0; j < input[i].entryNum; j++){
			fread(&buf, 2, 1, input[i].fp); // エントリタグをチェック
			switch (buf){
			case 0x0100:	// 画像の横幅
				// データ位置まで移動
				fseek(input[i].fp, 6, SEEK_CUR);
				fread(&input[i].imgWidth, 4, 1, input[i].fp);
				break;
			case 0x0101:	// 画像の縦幅
				// データ位置まで移動
				fseek(input[i].fp, 6, SEEK_CUR);
				fread(&input[i].imgHeight, 4, 1, input[i].fp);
				break;
			case 0x0111:	// 画像データの開始位置を格納している場合
				// データ位置まで移動
				fseek(input[i].fp, 6, SEEK_CUR);
				// 画像データの開始位置を得る
				fread(&input[i].imgPos, 4, 1, input[i].fp);\
				break;
			default:
				fseek(input[i].fp, 10, SEEK_CUR); // 次のエントリダグまで移動
				break;
			}
		}
	}
	
	/* ---------- 画像データの開始位置が存在するかチェック ----------- */
	for(i = 0; i < 2; i++){
		if(input[i].imgPos == 0){
			printf("image position error\n");
			printf("%d, %lx ", i, input[i].imgPos);
			return 1;
		}
	}
	
	/* ---------- 画像データ解像度同一チェック ----------- */
	if(input[0].imgHeight != input[1].imgHeight || input[0].imgWidth != input[1].imgWidth){
		printf("image size error\n");
		printf("%d %d %d %d \n", input[0].imgHeight, input[1].imgHeight, input[0].imgWidth, input[1].imgWidth);
		return 1;
	}
	
	// 総画素数を計算
	pixNum = input[0].imgHeight * input[0].imgWidth;
	
	/* --------- 画像処理部分 --------- */

	// 画像データ開始位置へ移動
	for(i = 0; i < 2; i++){
		fseek(input[i].fp, input[i].imgPos, SEEK_SET);
	}
	fseek(fpw, input[0].imgPos, SEEK_SET);
	
	//各ピクセルごとの操作
	for(i = 0; i < pixNum; i++){
		for(j = 0; j < 3; j++){	// ピクセルのRGBの各数値を取得
			fread(&input[0].pixel[j], bitNum, 1, input[0].fp);
			fread(&input[1].pixel[j], bitNum, 1, input[1].fp);
		}
		// 2つの入力ファイルの輝度をそれぞれ計算して比較
		if(culc(input[0].pixel[0], input[0].pixel[1], input[0].pixel[2]) < culc(input[1].pixel[0], input[1].pixel[1], input[1].pixel[2])){
			for(j = 0; j < 3; j++){
				fwrite(&input[1].pixel[j], bitNum, 1, fpw);
			}
		}else{
			fseek(fpw, 3, SEEK_CUR);
		}
	}
	
	for(i = 0; i < 2; i++){
		fclose(input[i].fp);
	}
	fclose(fpw);
	
	return 0;
}