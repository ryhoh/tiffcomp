// 堀哲也 2016/11/05

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

// NUM表示のないシンプルなバージョンも用意する
int simpleProgress(int pos, int full){
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
	printf("|%5.1f%%\n", per);
	
	return 0;
}