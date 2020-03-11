// Copyright (c) 2017-2019 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RPC_MISC_H
#define BITCOIN_RPC_MISC_H

#include <script/script.h>

class CWallet;
class UniValue;

CScript createmultisig_redeemScript(CWallet *const pwallet,
                                    const UniValue &params);

#endif // BITCOIN_RPC_MISC_H
