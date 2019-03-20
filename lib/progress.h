// Copyright (c) 2016-2019 Tetsuya Hori
// Released under the MIT license
// https://opensource.org/licenses/mit-license.php

// 修正 - 2019/03/17 堀哲也

/*		int printProgress(PROGRESS_STATE state);
 *
 *		表示例	（全工程1000個中、334個完了の状態）
 *
 *		|******--------------| 33.4%    NUM:[ 334 / 1000 ]
 *
 *		表示部は20文字からなる21段階で、5%完了する毎に*が追加される
 *		百分率は右詰め5桁（.以下1桁）表記、空きは空白で埋める
 */

#ifndef PROGRESS_H
#define PROGRESS_H

#include <string>

namespace ryhoh_prgr
{
    class Progress
    {
    private:
        int max;  /* 全作業工程の数 */
        int pos;  /* 完了した作業工程の数 */
        /* 常に 0 <= pos <= max を満たすこと */

    public:
        Progress(int max);
        virtual ~Progress();
        virtual std::string generateString();
        void click();
        int getMax();
        int getPos();
    };

    const int BAR_LENGTH = 20;
    const char DONE_CHAR = '*';
    const char YET_CHAR  = '-';

    inline int Progress::getMax() {return this->max;}
    inline int Progress::getPos() {return this->pos;}


    // NUM表示のないシンプルなバージョン
    class SimpleProgress: public Progress
    {
    public:
        // SimpleProgress(int max);
        using Progress::Progress;
        virtual ~SimpleProgress();
        std::string generateString() override;
    };
}

#endif
