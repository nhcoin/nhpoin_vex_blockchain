using eosio::check;
namespace nhp {

     void nhpoint::create(eosio::name issuer,
     eosio::asset maximum_supply) {
     require_auth(_self);
     
     auto sym = maximum_supply.symbol;
     check(sym.is_valid(), "invalid symbol name");
     check(maximum_supply.is_valid(), "invalid supply");
     check(maximum_supply.amount > 0, "max-supply must be positive");

