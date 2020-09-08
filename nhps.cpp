#include "nhp.hpp"
#include "nhp.token.cpp"
using eosio::check;
namespace nhp {
    void nhpoint::inflation(eosio::asset addsupply) {
        require_auth(_self);
        auto sym = addsupply.symbol;
        check(sym.is_valid(), "invalid symbol name");
        check(addsupply.is_valid(), "invalid supply");
        check(addsupply.amount > 0, "add-supply must be positive");
        
        stats statstable(_self, sym.code().raw());
        auto itr = statstable.find( sym.code().raw() );
        check(itr != statstable.end(), "token with symbol does not exists");
        check(false == itr->paused, "token with symbol been paused");
        
        statstable.modify(itr, _self, [&](auto &s) {
        s.max_supply += addsupply;
        });
    }
 
    void nhpoint::subsupply(eosio::asset subsupply) {
        require_auth(_self);
        auto sym = subsupply.symbol;
        check(sym.is_valid(), "invalid symbol name");
        check(subsupply.is_valid(), "invalid supply");
        check(subsupply.amount > 0, "add-supply must be positive");
        
        stats statstable(_self, sym.code().raw());
        auto itr = statstable.find( sym.code().raw() );
        check(itr != statstable.end(), "token with symbol does not exists");
        check(false == itr->paused, "token with symbol been paused");
        statstable.modify(itr, _self, [&](auto &s) {
        s.max_supply -= subsupply;
        });
    }
 
    void nhpoint::retire(eosio::name from, eosio::asset quantity, std::string memo) {
        auto sym = quantity.symbol;
        check( sym.is_valid(), "invalid symbol name" );
        check( memo.size() <= 256, "memo has more than 256 bytes" );
        
        stats statstable( get_self(), sym.code().raw() );
        auto existing = statstable.find( sym.code().raw() );
        check( existing != statstable.end(), "token with symbol does not exist" );
        
        check(false == existing->paused, "token with symbol been paused");
        const auto& st = *existing;
        
        // require_auth( st.issuer );
        // if (st.issuer != from)
        {
            require_auth(from);
        }
 
        check( quantity.is_valid(), "invalid quantity" );
        check( quantity.amount > 0, "must retire positive quantity" );
        
        check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

        statstable.modify( st, eosio::same_payer, [&]( auto& s ) {
        s.supply -= quantity;
        });
        sub_balance(from, quantity);
     }
 
    void nhpoint::pause(eosio::symbol symbol) {
        require_auth(_self);
        
        stats statstable(_self, symbol.code().raw());
        auto itr = statstable.find(symbol.code().raw());
        check(itr != statstable.end(), "token with symbol does not exists");
        check(false == itr->paused, "token with symbol have been paused");
        statstable.modify(itr, _self, [&](auto &s) {
        s.paused = true;
        });
    }
    
    void nhpoint::unpause(eosio::symbol symbol) {
        require_auth(_self);
        stats statstable(_self, symbol.code().raw());
        auto itr = statstable.find(symbol.code().raw());
        check(itr != statstable.end(), "token with symbol does not exists");
        check(true == itr->paused, "token with symbol have been unpaused");
        statstable.modify(itr, _self, [&](auto &s) {
        s.paused = false;
        });
    }
    
    void nhpoint::freeze(eosio::name owner) {
        require_auth(_self);
        frozen_table frozentable(_self, _self.value);
        auto itr = frozentable.find(owner.value);
        check(itr == frozentable.end(), "token owner have been freezed");
        frozentable.emplace(_self, [&](auto &s) {
        s.owner = owner;
        });
    }
    
    void nhpoint::unfreeze(eosio::name owner) {
        require_auth(_self);
        frozen_table frozentable(_self, _self.value);
        auto itr = frozentable.find(owner.value);
        check(itr != frozentable.end(), "not find token owner");
        frozentable.erase(itr);
    }
};

EOSIO_DISPATCH(nhp::nhpoint,
(create)(open)(close)(issue)(transfer)(retire)(addsupply)(subsupply)(pause)(unpause)(freeze)(unfreeze)
)
