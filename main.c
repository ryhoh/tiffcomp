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

#include <stdio.h>
#include "tiffcomp.h"
#include "progressbar.h"

// 出力されたファイルを再度読み込むために一度リネームする（中間ファイルとなる）
#define TEMPFILE "temp.tif"	// 処理を連続して行うためにこのファイル名に変更する

int main(int argc, char *argv[]){
	int i = 0, num = argc - 1;
	/*
	 * num 総ファイル数
	 * i 合成処理済みのファイル数
	 */
	
	/* 合成ファイルは2枚以上必要 */
	if(num < 2){
		printf("lack of file error\n");
		return 1;
	}
	
	/* 作業開始前に1度プログレスバーを見せておく */
	if(printProgress(i, num))
		return 1;
	
	 /* ------画像合成処理------ */
	// まず1枚目と2枚目
	if(comp2(argv[1], argv[2])){	// ファイル名はargv[1] からargv[num-1]まで
		printf("comp error\n");
		return 1;
	}
	
	/* 2枚処理したのでi = 2として処理を続ける */
	for(i = 2; i < num; i++){
		if(printProgress(i, num))		// プログレスバーの更新を行う
			return 1;
		if(rename(OUTPUTFILE, TEMPFILE)){	// 続けて処理する場合、出力されたファイルを再度読み込むために一度リネームする
			printf("name change error\n");
			return 1;
		}
		if(comp2(TEMPFILE, argv[i+1])){	// ファイル名はargv[1] からargv[num-1]まで
			printf("comp error\n");
			return 1;
		}
	}
	
	if(printProgress(i, num))		// 最後にもう一度プログレスバーの更新を行う
		return 1;
	
	if(2 < num){	// 3枚以上の合成の後は中間ファイルがゴミとして残るので削除する
		if(remove(TEMPFILE)){
			printf("temporary file removing error\n");
			printf("remove %s manually\n", TEMPFILE);
		}
	}
	
	return 0;
}