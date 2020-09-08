using eosio::check;
namespace nhp {

     void nhpoint::create(eosio::name issuer,
          eosio::asset maximum_supply) {
          require_auth(_self);
     
          auto sym = maximum_supply.symbol;
          check(sym.is_valid(), "invalid symbol name");
          check(maximum_supply.is_valid(), "invalid supply");
          check(maximum_supply.amount > 0, "max-supply must be positive");

          stats statstable(_self, sym.code().raw());
          auto existing = statstable.find(sym.code().raw());
          check(existing == statstable.end(), "token with symbol already exists");  
          
          statstable.emplace(_self, [&](auto &s) {
          s.supply.symbol = maximum_supply.symbol;
          s.max_supply = maximum_supply;
          s.issuer = issuer;
          s.paused = false;
          });
     }  
     void nhpoint::issue(eosio::name to, eosio::asset quantity, std::string memo)
{
 
     auto sym = quantity.symbol;
     check( sym.is_valid(), "invalid symbol name" );
     check( memo.size() <= 256, "memo has more than 256 bytes" );
          
     stats statstable( _self, sym.code().raw() );
     auto existing = statstable.find( sym.code().raw() );
     check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
     check(false == existing->paused, "token with symbol been paused");
     const auto& st = *existing;
          
     require_auth( st.issuer );
     check( quantity.is_valid(), "invalid quantity" );
     check( quantity.amount > 0, "must issue positive quantity" );
     check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
     check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
     statstable.modify( st, eosio::same_payer, [&]( auto& s ) {
     s.supply += quantity;
     });
     
     add_balance( st.issuer, quantity, st.issuer );
     if( to != st.issuer ) { SEND_INLINE_ACTION( *this, transfer, { {st.issuer, "active"_n} },
     { st.issuer, to, quantity, memo }
     );
     }
 }
     void nhpoint::transfer(eosio::name from, 
               eosio::name to,
               eosio::asset quantity,
               std::string memo) {
     check(from != to, "cannot transfer to self");
     require_auth(from);
     check(is_account(to), "to account does not exist");
     auto sym = quantity.symbol.code().raw();
     stats statstable(_self, sym);
     const auto &st = statstable.get(sym);
     check(false == st.paused, "token been paused");
     frozen_table frozentable(_self, _self.value);
     auto itrf = frozentable.find(from.value);
     check(itrf == frozentable.end(), "from account have been freezed");
     auto itrt = frozentable.find(to.value);
     check(itrt == frozentable.end(), "to account have been freezed");
     require_recipient(from);
     require_recipient(to);
     check(quantity.is_valid(), "invalid quantity");
     check(quantity.amount > 0, "must transfer positive quantity");
     check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
     check(memo.size() <= 256, "memo has more than 256 bytes");
     auto payer = has_auth(to) ? to : from;
     sub_balance(from, quantity);
     add_balance(to, quantity, payer);
 }
     
     void nhpoint::sub_balance(eosio::name owner, eosio::asset value) {
     accounts from_acnts(_self, owner.value);
     const auto &from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
     check(from.balance.amount >= value.amount, "overdrawn balance");
     if (from.balance.amount == value.amount) {
         from_acnts.erase(from);
     } else {
          from_acnts.modify(from, owner, [&](auto &a) { a.balance -= value;
          });
     }
 }
     void nhpoint::add_balance(eosio::name owner, eosio::asset value, eosio::name
     ram_payer) {
     accounts to_acnts(_self, owner.value);
     auto to = to_acnts.find(value.symbol.code().raw());
     if (to == to_acnts.end()) {
     to_acnts.emplace(ram_payer, [&](auto &a) { a.balance = value;
     });
     } else {
          to_acnts.modify(to, eosio::same_payer, [&](auto &a) {
               a.balance += value;
          });
     }
 }
     eosio::asset nhpoint::get_supply(eosio::symbol sym) const {
     stats statstable(_self, sym.code().raw());
     const auto &st = statstable.get(sym.code().raw());
     return st.supply;
     }
     eosio::asset nhpoint::get_balance(eosio::name owner, eosio::symbol sym) const
     {
     accounts accountstable(_self, owner.value);
     const auto &ac = accountstable.get(sym.code().raw());
     return ac.balance;
 }
     
     void nhpoint::open(const eosio::name &owner, const eosio::symbol &symbol, const eosio::name &ram_payer) {
     require_auth(ram_payer);
          
     check(is_account(owner), "owner account does not exist");
          
     auto sym_code_raw = symbol.code().raw();
     stats statstable(get_self(), sym_code_raw);
     const auto &st = statstable.get(sym_code_raw, "symbol does not exist");
     check(false == st.paused, "token been paused");
     check(st.supply.symbol == symbol, "symbol precision mismatch");
          
     accounts acnts(get_self(), owner.value);
     auto it = acnts.find(sym_code_raw);
     if (it == acnts.end()) {
          acnts.emplace(ram_payer, [&](auto &a) {
               a.balance = eosio::asset{0, symbol};
               });
     }
 }

     void nhpoint::close(const eosio::name &owner, const eosio::symbol &symbol) {
     require_auth(owner);
     accounts acnts(get_self(), owner.value);
     auto it = acnts.find(symbol.code().raw());
     check(it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect.");
     check(it->balance.amount == 0, "Cannot close because the balance is not zero.");
     acnts.erase(it);
     }
} /// namespace eosio
   
