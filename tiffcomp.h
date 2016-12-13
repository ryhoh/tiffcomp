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
#IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

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