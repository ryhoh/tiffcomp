/*
 *		progressbar.c
 *
 *		int pos		完了した作業工程の数
 *		int full	全作業工程の数
 *
 *		入力されるposは常に		0 <= pos <= full	を満たす
 *
 *		表示成功で0、失敗で1を返す
 *
 *
 *		表示例	（全工程1000個中、334個完了の状態）
 *
 *		|******--------------| 33.4%    NUM:[ 334 / 1000 ]
 *
 *		表示部は20文字からなる21段階で、5%完了する毎に*が追加される
 *		百分率は右詰め5桁（.以下1桁）表記、空きは空白で埋める
 *
 */

#include <stdio.h>

int printProgress(int pos, int full){
	int i, bar;
	float per = (float)pos / full * 100;	// 進行状況を小数点で保持する
	
	/* 不正なposを入力した場合は異常終了する */
	if(pos < 0 || full < pos){
		printf("progress bar error\n");
		return 1;
	}
	
	/* 21段階のプログレスバーを表示するにあたって、fullに対するposの割合を0から20の21段階で表したい */
	bar = pos * 20 / full;	// 0 <= pos <= 20	（但しposは整数）
	
	
	/* 表示部 */
	printf("|");
	
	for(i = 0; i < bar; i++){
		printf("*");
	}
	
	for(i = bar; i < 20; i++){
		printf("-");
	}
	
	printf("|%5.1f%%    NUM:[ %d / %d ]\n", per, pos, full);
	
	return 0;
}