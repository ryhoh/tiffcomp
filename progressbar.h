/*		int printProgress(int pos, int full)
 *
 *		int pos		完了した作業工程の数
 *		int full	全作業工程の数
 *
 *		入力されるposは常に		0 <= pos <= full	を満たす
 *		表示成功で0、失敗で1を返す
 *
 *
 *		表示例	（全工程1000個中、334個完了の状態）
 *
 *		|******--------------| 33.4%    NUM:[ 334 / 1000 ]
 *
 *		表示部は20文字からなる21段階で、5%完了する毎に*が追加される
 *		百分率は右詰め5桁（.以下1桁）表記、空きは空白で埋める
 */

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

int printProgress(int pos, int full);

// NUM表示のないシンプルなバージョン
int simpleProgress(int pos, int full); 

#endif