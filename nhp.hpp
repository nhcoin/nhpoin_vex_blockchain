#pragma once
#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

 namespace nhp {

 class [[eosio::contract("nhp")]] nhpoint :public eosio::contract {

 public:
 nhpoint(eosio::name receiver, eosio::name code, eosio::datastream<
 const char *> ds) :
 eosio::contract(receiver, code, ds) {
 }

 ACTION create(eosio::name issuer,eosio::asset maximum_supply);

 ACTION open(const eosio::name &owner, const eosio::symbol &symbol,
 const eosio::name &ram_payer);
 ACTION close(const eosio::name &owner, const eosio::symbol &symbol
 );

 ACTION issue(eosio::name to, eosio::asset quantity, std::string memo);

 ACTION transfer(eosio::name from,
 eosio::name to,
 eosio::asset quantity,
 std::string memo);

 inline eosio::asset get_supply(eosio::symbol sym) const;

 inline eosio::asset get_balance(eosio::name owner, eosio::symbol sym) const;

 TABLE account {
 eosio::asset balance;

 uint64_t primary_key() const { return balance.symbol.code().raw(); }
 };

 TABLE currency_stats {
 eosio::asset supply;
 eosio::asset max_supply;
 eosio::name issuer;

 bool paused;

 uint64_t primary_key() const { return supply.symbol.code().raw(); }
 };

 typedef eosio::multi_index<"accounts"_n, account> accounts;
 typedef eosio::multi_index<"stat"_n, currency_stats> stats;

 ACTION retire(eosio::name from,
 eosio::asset quantity,
 std::string memo);

 ACTION fixstat(eosio::symbol symbol);

 ACTION inflation(eosio::asset addsupply);
 ACTION subsupply(eosio::asset subsupply);
 ACTION pause(eosio::symbol symbol);
 ACTION unpause(eosio::symbol symbol);
 ACTION freeze(eosio::name owner);
 ACTION unfreeze(eosio::name owner);
 
 protected:
 void sub_balance(eosio::name owner, eosio::asset value);
 void add_balance(eosio::name owner, eosio::asset value, eosio::name ram_payer);
 
 public:
 struct transfer_args {
 eosio::name from;
 eosio::name to;
 eosio::asset quantity;
 std::string memo;
 };

 TABLE frozen_account
 {
 eosio::name owner;
 uint64_t primary_key() const { return owner.value; }
 };
 typedef eosio::multi_index<"frozen"_n, frozen_account> frozen_table;
 };

} /// namespace eosio
