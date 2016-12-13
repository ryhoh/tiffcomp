#ifndef TIFFCOMP_H
#define TIFFCOMP_H

#define OUTPUTFILE "output.tif"	// 出力するファイルの名前
#define bitNum 1 	// 1Byte = 8bitのTIFFを想定

struct tiff {
	FILE *fp;				/* ファイルポインタ */
	int idfPos;				/* IDF情報の開始位置 */
	int entryNum;			/* エントリカウント数 */
	int imgHeight;			/* 画像の縦幅 */
	int imgWidth;			/* 画像の横幅 */
	long imgPos;			/* 画像データの開始位置 */
	int pixel[3];			/* 画素比較用ピクセルデータ */
};

unsigned long culc(int a, int b, int c);
int comp2(char file1[], char file2[]);
/* int comp2(char file1[], char file2[]);
 * ファイル名（パス）2つを引数とし、合成の手続きを行う
 * 成功時に0、失敗時に1が返る
 */

#endif