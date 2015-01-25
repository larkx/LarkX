#include <bts/blockchain/time.hpp>
#include <bts/db/level_map.hpp>
#include <bts/wallet/wallet_db.hpp>

#include <fc/io/json.hpp>
#include <fstream>

namespace bts { namespace wallet {

   using namespace bts::blockchain;

   namespace detail {
     class wallet_db_impl
     {
        public:
           wallet_db*                                        self = nullptr;
           bts::db::level_map<int32_t,generic_wallet_record> _records;

           void store_generic_record( const generic_wallet_record& record, bool sync = true )
           { try {
               auto index = record.get_wallet_record_index();
               FC_ASSERT( index != 0 );
               FC_ASSERT( _records.is_open() );
               _records.store( index, record, sync );
               load_generic_record( record );
           } FC_CAPTURE_AND_RETHROW( (record) ) }

           void load_generic_record( const generic_wallet_record& record, bool overwrite = true )
           { try {
               switch( wallet_record_type_enum(record.type) )
               {
                   case master_key_record_type:
                       load_master_key_record( record.as<wallet_master_key_record>(), overwrite );
                       break;
                   case account_record_type:
                       load_account_record( record.as<wallet_account_record>(), overwrite );
                       break;
                   case key_record_type:
                       load_key_record( record.as<wallet_key_record>(), overwrite );
                       break;
                   case transaction_record_type:
                       load_transaction_record( record.as<wallet_transaction_record>(), overwrite );
                       break;
                   case balance_record_type:
                       load_balance_record( record.as<wallet_balance_record>(), overwrite );
                       break;
                   case property_record_type:
                       load_property_record( record.as<wallet_property_record>(), overwrite );
                       break;
                   case setting_record_type:
                       load_setting_record( record.as<wallet_setting_record>(), overwrite );
                       break;
                   default:
                       elog( "Unknown wallet record type: ${type}", ("type",record.type) );
                       break;
                }
           } FC_CAPTURE_AND_RETHROW( (record) ) }

           void load_master_key_record( const wallet_master_key_record& key, bool overwrite )
           { try {
              if( !overwrite) FC_ASSERT( !self->wallet_master_key.valid() );
              self->wallet_master_key = key;
           } FC_CAPTURE_AND_RETHROW() }

           void load_account_record( const wallet_account_record& account_to_load, bool overwrite )
           { try {
              self->accounts[account_to_load.wallet_record_index] = account_to_load;

              auto current_index_itr = self->address_to_account_wallet_record_index.find( account_to_load.account_address );
              if( !overwrite) FC_ASSERT( current_index_itr == self->address_to_account_wallet_record_index.end() );
              self->address_to_account_wallet_record_index[ account_to_load.account_address ]= account_to_load.wallet_record_index;
              for( const auto& active_key : account_to_load.active_key_history )
                self->address_to_account_wallet_record_index[ address(active_key.second) ]= account_to_load.wallet_record_index;

              if( account_to_load.id != 0 )
              {
                auto current_account_id_itr = self->account_id_to_wallet_record_index.find( account_to_load.id );
                if( current_account_id_itr != self->account_id_to_wallet_record_index.end() )
                    FC_ASSERT( current_account_id_itr->second == account_to_load.wallet_record_index, "",
                                ("current_account_id_tr",*current_account_id_itr)
                                ("account_to_load.index",account_to_load.wallet_record_index) );
                self->account_id_to_wallet_record_index[ account_to_load.id ] = account_to_load.wallet_record_index;
              }

              auto current_name_itr = self->name_to_account_wallet_record_index.find( account_to_load.name );
              if( !overwrite) FC_ASSERT( current_name_itr == self->name_to_account_wallet_record_index.end() );
              self->name_to_account_wallet_record_index[ account_to_load.name ] = account_to_load.wallet_record_index;

           } FC_CAPTURE_AND_RETHROW( (account_to_load) ) }

           void load_key_record( const wallet_key_record& key_to_load, bool overwrite )
           { try {
              address key_address( key_to_load.public_key );
              auto current_key_itr = self->keys.find( key_address );
              if( !overwrite) FC_ASSERT( current_key_itr == self->keys.end(), "Key should be unique!" );

              self->keys[key_address] = key_to_load;

              auto key = key_to_load.public_key;
              auto bts_addr = key_to_load.get_address();
              self->btc_to_bts_address[ address(key) ] = bts_addr;
              self->btc_to_bts_address[ address(pts_address(key,false,28) )] = bts_addr;
              self->btc_to_bts_address[ address(pts_address(key,true,28) ) ] = bts_addr;
              self->btc_to_bts_address[ address(pts_address(key,false,56) )] = bts_addr;
              self->btc_to_bts_address[ address(pts_address(key,true,56) ) ] = bts_addr;
              self->btc_to_bts_address[ address(pts_address(key,false,0) ) ] = bts_addr;
              self->btc_to_bts_address[ address(pts_address(key,true,0) )  ] = bts_addr;
           } FC_CAPTURE_AND_RETHROW( (key_to_load) ) }

           void load_transaction_record( const wallet_transaction_record& rec, bool overwrite )
           { try {
              auto itr = self->transactions.find( rec.record_id );
              if( !overwrite) FC_ASSERT( itr == self->transactions.end(), "Duplicate transaction found in wallet!" );
              self->transactions[ rec.record_id ] = rec;
           } FC_CAPTURE_AND_RETHROW( (rec) ) }

           void load_balance_record( const wallet_balance_record& rec, bool overwrite )
           { try {
              if( !overwrite )
              {
                  auto itr = self->balances.find( rec.id() );
                  FC_ASSERT( itr == self->balances.end(), "Duplicate balance record" );
              }
              self->balances[ rec.id() ] = rec;
           } FC_CAPTURE_AND_RETHROW( (rec) ) }

           void load_property_record( const wallet_property_record& property_rec, bool overwrite )
           { try {
              if( !overwrite )
              {
                  auto itr = self->properties.find( property_rec.key );
                  FC_ASSERT( itr == self->properties.end(), "Duplicate Property Record" );
              }
              self->properties[property_rec.key] = property_rec;
           } FC_CAPTURE_AND_RETHROW( (property_rec) ) }

           void load_setting_record( const wallet_setting_record& rec, bool overwrite )
           { try {
              self->settings[rec.name] = rec;
           } FC_CAPTURE_AND_RETHROW( (rec) ) }
     };

   } // namespace detail


   wallet_db::wallet_db()
   :my( new detail::wallet_db_impl() )
   {
      my->self = this;
   }

   wallet_db::~wallet_db()
   {
   }

   void wallet_db::open( const fc::path& wallet_file )
   { try {
      try
      {
          my->_records.open( wallet_file, true );
          for( auto itr = my->_records.begin(); itr.valid(); ++itr )
          {
             auto record = itr.value();
             try
             {
                my->load_generic_record( record );
                // prevent hanging on large wallets
                fc::usleep( fc::microseconds(1000) );
             }
             catch (const fc::canceled_exception&)
             {
                throw;
             }
             catch ( const fc::exception& e )
             {
                wlog( "Error loading wallet record:\n${r}\nreason: ${e}", ("e",e.to_detail_string())("r",record) );
             }
          }
      }
      catch( ... )
      {
          close();
          throw;
      }
   } FC_RETHROW_EXCEPTIONS( warn, "Error opening wallet file ${file}", ("file",wallet_file) ) }

   void wallet_db::close()
   {
      my->_records.close();

      wallet_master_key.reset();

      accounts.clear();
      keys.clear();
      transactions.clear();
      balances.clear();
      properties.clear();
      settings.clear();

      btc_to_bts_address.clear();
      address_to_account_wallet_record_index.clear();
      account_id_to_wallet_record_index.clear();
      name_to_account_wallet_record_index.clear();
   }

   bool wallet_db::is_open()const { return my->_records.is_open(); }

   void wallet_db::store_generic_record( const generic_wallet_record& record, bool sync )
   {
       FC_ASSERT( is_open() );
       my->store_generic_record( record, sync );
   }

   int32_t wallet_db::new_wallet_record_index()
   {
      auto next_rec_num = get_property( next_record_number );
      int32_t next_rec_number = 2;
      if( next_rec_num.is_null() )
      {
         next_rec_number = 2;
      }
      else
      {
         next_rec_number = next_rec_num.as<int32_t>();
      }
      set_property( property_enum::next_record_number, next_rec_number + 1, false );
      return next_rec_number;
   }

   int32_t wallet_db::new_key_child_index( const address& account_address )
   {
      owallet_account_record account_rec = lookup_account( account_address );
      if( account_rec )
      {
          account_rec->last_used_gen_sequence++;
          cache_account( *account_rec );
          return account_rec->last_used_gen_sequence;
      }

      auto next_child_idx = get_property( next_child_key_index );
      int32_t next_child_index = 0;
      if( next_child_idx.is_null() )
      {
         next_child_index = 1;
      }
      else
      {
         next_child_index = next_child_idx.as<int32_t>();
      }
      set_property( property_enum::next_child_key_index, next_child_index + 1 );
      return next_child_index;
   }

   private_key_type wallet_db::get_private_key( const fc::sha512& password,
                                                int index )
   {
      FC_ASSERT( wallet_master_key.valid() );

      const auto master_ext_priv_key = wallet_master_key->decrypt_key( password );
      const auto new_priv_key = master_ext_priv_key.child( index );

      return new_priv_key;
   }

   private_key_type wallet_db::new_private_key( const fc::sha512& password,
                                                const address& parent_account_address,
                                                bool store_key )
   {
      FC_ASSERT( wallet_master_key.valid() );

      const auto master_ext_priv_key = wallet_master_key->decrypt_key( password );
      const auto key_index = new_key_child_index( parent_account_address );
      auto new_priv_key = master_ext_priv_key.child( key_index );
      if( key_index >= 10000 )
      {
          fc::sha256::encoder enc;
          fc::raw::pack( enc, parent_account_address );
          fc::raw::pack( enc, key_index );
          new_priv_key = master_ext_priv_key.child( enc.result() );
      }

      if( !store_key )
        return new_priv_key;

      key_data new_key;
      new_key.account_address = parent_account_address;
      new_key.encrypt_private_key( password, new_priv_key );
      new_key.gen_seq_number = key_index;

      // if there is no parent account address, then the account_address of this key is itself
      if( parent_account_address == address() )
         new_key.account_address = address( new_key.public_key );

      this->store_key( new_key );
      return new_priv_key;
   }

   void wallet_db::set_property( const property_enum property_id, const variant& v, const bool sync )
   {
      wallet_property_record property_record;
      auto property_itr = properties.find( property_id );
      if( property_itr != properties.end() )
      {
          property_record = property_itr->second;
          property_record.value = v;
      }
      else
      {
          if( property_id == property_enum::next_record_number )
              property_record = wallet_property_record( wallet_property(property_id, v), 1 );
          else
              property_record = wallet_property_record( wallet_property(property_id, v), new_wallet_record_index() );
      }
      store_record( property_record, sync );
   }

   variant wallet_db::get_property( const property_enum property_id )const
   {
      auto property_itr = properties.find( property_id );
      if( property_itr != properties.end() ) return property_itr->second.value;
      return variant();
   }

   string wallet_db::get_account_name( const address& account_address )const
   {
      auto opt = lookup_account( account_address );
      if( opt ) return opt->name;
      return "?";
   }

   owallet_account_record wallet_db::lookup_account( account_id_type aid )const
   {
      auto itr = account_id_to_wallet_record_index.find(aid);
      if( itr != account_id_to_wallet_record_index.end() )
      {
         auto aitr = accounts.find( itr->second );
         FC_ASSERT( accounts.end() != aitr );
         return aitr->second;
      }
      return owallet_account_record();
   }

   owallet_transaction_record wallet_db::lookup_transaction( const transaction_id_type& record_id )const
   {
      auto itr = transactions.find( record_id );
      if( itr != transactions.end() ) return itr->second;
      return owallet_transaction_record();
   }

   void wallet_db::store_setting(const string& name, const variant& value)
   {
       auto orec = lookup_setting(name);
       if (orec.valid())
       {
           orec->value = value;
           settings[name] = *orec;
           store_record( *orec );
       }
       else
       {
           auto rec = wallet_setting_record( setting(name, value), new_wallet_record_index() );
           settings[name] = rec;
           store_record( rec );
       }
   }

   void wallet_db::store_key( const key_data& key_to_store )
   { try {
      auto key_itr = keys.find( key_to_store.get_address() );
      if( key_itr != keys.end() )
      {
         key_data& old_data = key_itr->second;
         old_data = key_to_store;

         if( key_to_store.has_private_key() )
         {
            auto oacct = lookup_account( key_to_store.account_address );
            if( oacct )
            {
              // FC_ASSERT(oacct.valid(), "expecting an account to existing at this point");
               oacct->is_my_account = true;
               cache_account( *oacct, false );
               /*
               ilog( "WALLET: storing private key for ${key} under account '${account_name}' address: (${account})",
                     ("key",key_to_store.public_key)
                     ("account",key_to_store.account_address)
                    ("account_name",get_account_name(key_to_store.account_address)) );
                    */
            }
         }
         else
         {
            /*
            ilog( "WALLET: storing public key ${key} under account named '${account_name}' address: (${account})",
                  ("key",key_to_store.public_key)
                  ("account",key_to_store.account_address)
                  ("account_name",get_account_name(key_to_store.account_address)) );
                  */
         }
         //ilog( "storing key" );

         store_record( key_itr->second, true );
      }
      else
      {
         auto r = wallet_key_record( key_to_store, new_wallet_record_index() );
         store_record( keys[ key_to_store.get_address() ] = r, true );

         auto key = key_to_store.public_key;
         auto bts_addr = key_to_store.get_address();
         btc_to_bts_address[ address( key ) ] = bts_addr;
         btc_to_bts_address[ address( pts_address( key, false, 56 ) ) ] = bts_addr;
         btc_to_bts_address[ address( pts_address( key, true, 56 ) ) ] = bts_addr;
         btc_to_bts_address[ address( pts_address( key, false, 0 ) ) ] = bts_addr;
         btc_to_bts_address[ address( pts_address( key, true, 0 ) ) ] = bts_addr;
         ilog( "indexing key ${k}", ("k",address( pts_address( key, false, 0 ) )) );
         ilog( "indexing key ${k}", ("k",address( pts_address( key, true, 56 ) )) );
      }
   } FC_CAPTURE_AND_RETHROW() }

   vector<wallet_transaction_record> wallet_db::get_pending_transactions()const
   {
       vector<wallet_transaction_record> transaction_records;
       for( const auto& item : transactions )
       {
           const auto& transaction_record = item.second;
           if( !transaction_record.is_virtual && !transaction_record.is_confirmed )
               transaction_records.push_back( transaction_record );
       }
       return transaction_records;
   }

   void wallet_db::export_to_json( const path& filename )const
   { try {
      FC_ASSERT( is_open() );
      FC_ASSERT( !fc::exists( filename ) );

      const auto dir = fc::absolute( filename ).parent_path();
      if( !fc::exists( dir ) )
          fc::create_directories( dir );

      std::ofstream fs( filename.string() );
      fs.write( "[\n", 2 );

      auto itr = my->_records.begin();
      while( itr.valid() )
      {
          auto str = fc::json::to_pretty_string( itr.value() );
          if( (++itr).valid() ) str += ",";
          str += "\n";
          fs.write( str.c_str(), str.size() );
      }

      fs.write( "]", 1 );
   } FC_CAPTURE_AND_RETHROW( (filename) ) }

   void wallet_db::import_from_json( const path& filename )
   { try {
      FC_ASSERT( fc::exists( filename ) );
      FC_ASSERT( is_open() );

      auto records = fc::json::from_file<std::vector<generic_wallet_record>>( filename );
      for( const auto& record : records )
         store_generic_record( record );
   } FC_CAPTURE_AND_RETHROW( (filename) ) }

   bool wallet_db::has_private_key( const address& addr )const
   { try {
      auto itr = keys.find( addr );
      if( itr != keys.end() )
      {
         return itr->second.has_private_key();
      }
      return false;
   } FC_CAPTURE_AND_RETHROW( (addr) ) }

   void wallet_db::cache_memo( const memo_status& memo,
                               const private_key_type& account_key,
                               const fc::sha512& password )
   {
      key_data data;
      data.account_address = address(account_key.get_public_key());
      //data.memo = memo_data(memo);
      data.valid_from_signature = memo.has_valid_signature;
      data.encrypt_private_key( password, memo.owner_private_key );

      store_key( data );
   }

   map<private_key_type, string> wallet_db::get_account_private_keys( const fc::sha512& password )const
   { try {
       map<public_key_type, string> public_keys;
       for( const auto& account_item : accounts )
       {
           const auto& account = account_item.second;
           public_keys[ account.owner_key ] = account.name;
           for( const auto& active_key_item : account.active_key_history )
           {
               const auto& active_key = active_key_item.second;
               public_keys[ active_key ] = account.name;
           }
       }

       map<private_key_type, string> private_keys;
       for( const auto& public_key_item : public_keys )
       {
           const auto& public_key = public_key_item.first;
           const auto& account_name = public_key_item.second;

           const auto key_record = lookup_key( public_key );
           if( !key_record.valid() || !key_record->has_private_key() )
               continue;

           try
           {
               private_keys[ key_record->decrypt_private_key( password ) ] = account_name;
           }
           catch( const fc::exception& e )
           {
               elog( "error decrypting private key: ${e}", ("e",e.to_detail_string()) );
           }
       }
       return private_keys;
   } FC_CAPTURE_AND_RETHROW() }

   owallet_balance_record wallet_db::lookup_balance( const balance_id_type& balance_id )const
   {
      auto itr = balances.find( balance_id );
      if( itr == balances.end() ) return fc::optional<wallet_balance_record>();
      return itr->second;
   }

   vector<wallet_balance_record> wallet_db::get_all_balances( const string& account_name, uint32_t limit )
   {
       auto ret = vector<wallet_balance_record>();
       uint32_t count = 0;
       for( auto item : balances )
       {
           if (count == limit && limit != uint32_t(-1))
               break;
           auto okey = lookup_key(item.second.owner());
           FC_ASSERT(okey.valid(), "expect a key record to exist at this point");
           auto oacct = lookup_account( okey->account_address );
           if ( oacct.valid() && oacct->name == account_name )
           {
               ret.push_back(item.second);
               count++;
           }
       }
       return ret;
   }

   owallet_key_record wallet_db::lookup_key( const address& address )const
   {
      auto btc_to_bts_itr = btc_to_bts_address.find( address );
      if( btc_to_bts_itr != btc_to_bts_address.end() )
      {
         auto itr = keys.find( btc_to_bts_itr->second ); //address );
         if( itr != keys.end() ) return itr->second;
      }
      return owallet_key_record();
   }

   owallet_setting_record wallet_db::lookup_setting( const string& name)const
   {
      auto itr = settings.find(name);
      if (itr != settings.end() )
      {
          return itr->second;
      }
      return owallet_setting_record();
   }

   void wallet_db::add_account( const account_record& blockchain_account,
                                const variant& private_data  )
   { try {
      wallet_account_record war;
      account_record& tmp  = war;
      tmp = blockchain_account;
      war.private_data = private_data;
      war.account_address = address(blockchain_account.owner_key);

      war.wallet_record_index = new_wallet_record_index();
      if (has_private_key(war.account_address))
          war.is_my_account = true;
      store_record( war, false );

      auto current_key = lookup_key( blockchain_account.owner_key );
      if( current_key.valid() )
      {
         current_key->account_address = address(blockchain_account.owner_key);
         store_record( *current_key, true );
      }
      else
      {
         wallet_key_record new_key;
         new_key.wallet_record_index = new_wallet_record_index();
         new_key.account_address = address(blockchain_account.owner_key);
         new_key.public_key = blockchain_account.owner_key;
         my->load_key_record( new_key, false );
         store_key( new_key );

         for( const auto& active_key : blockchain_account.active_key_history )
         {
           if( active_key.second != blockchain_account.owner_key )
           {
             new_key.wallet_record_index = new_wallet_record_index();
             new_key.public_key = active_key.second;
             my->load_key_record( new_key, false );
             store_key( new_key );
           }
         }
      }
   } FC_CAPTURE_AND_RETHROW( (blockchain_account) ) }

   void wallet_db::add_account( const string& new_account_name,
                                const public_key_type& new_account_key,
                                const variant& private_data )
   {
      auto current_account_itr = name_to_account_wallet_record_index.find( new_account_name );
      FC_ASSERT( current_account_itr == name_to_account_wallet_record_index.end(),
                 "Account with name ${name} already exists",
                 ("name",new_account_name) );
      auto current_address_itr = address_to_account_wallet_record_index.find( new_account_key );
      FC_ASSERT( current_address_itr == address_to_account_wallet_record_index.end(),
                 "Account with address ${address} already exists",
                 ("name",new_account_key) );

      wallet_account_record war;
      war.name = new_account_name;
      war.id = 0;
      war.account_address = address( new_account_key );
      war.owner_key = new_account_key;
      war.set_active_key( blockchain::now(), new_account_key );
      war.private_data = private_data;

      war.wallet_record_index = new_wallet_record_index();
      if (has_private_key(war.account_address))
          war.is_my_account = true;
      store_record( war );

      auto current_key = lookup_key( new_account_key );
      if( current_key )
      {
         current_key->account_address = address(new_account_key);
         store_record( *current_key, true );
      }
      else
      {
         wallet_key_record new_key;
         new_key.wallet_record_index = new_wallet_record_index();
         new_key.account_address = address(new_account_key);
         new_key.public_key = new_account_key;
         my->load_key_record( new_key, false );
         store_key( new_key );
      }
   }

   owallet_account_record wallet_db::lookup_account( const address& address_of_public_key )const
   {
      auto address_index_itr = address_to_account_wallet_record_index.find( address_of_public_key );
      if( address_index_itr != address_to_account_wallet_record_index.end() )
      {
         auto account_itr = accounts.find( address_index_itr->second );
         FC_ASSERT( account_itr != accounts.end(),
                    "wallet database in inconsistant state" );
         return account_itr->second;
      }
      return owallet_account_record();
   }

   owallet_account_record wallet_db::lookup_account( const string& account_name )const
   {
      auto name_index_itr = name_to_account_wallet_record_index.find( account_name );
      if( name_index_itr != name_to_account_wallet_record_index.end() )
      {
         auto account_itr = accounts.find( name_index_itr->second );
         FC_ASSERT( account_itr != accounts.end(),
                    "wallet database in inconsistant state" );
         return account_itr->second;
      }
      return owallet_account_record();
   }

   void wallet_db::cache_balance( const bts::blockchain::balance_record& balance_to_cache, const bool sync )
   {
      auto balance_id = balance_to_cache.id();
      owallet_balance_record current_bal = lookup_balance(balance_id);
      wallet_balance_record balance_record;
      if( !current_bal.valid() )
      {
         balance_record = wallet_balance_record( balance_to_cache, new_wallet_record_index() );
      }
      else
      {
         blockchain::balance_record& chain_rec = *current_bal;
         chain_rec = balance_to_cache;
         balance_record = *current_bal;
      }

      store_record( balance_record, sync );
   }

   void wallet_db::remove_contact_account( const string& account_name )
   {
      const auto opt_account = lookup_account( account_name );
      FC_ASSERT( opt_account.valid() );
      const auto acct = *opt_account;
      FC_ASSERT( ! has_private_key(address(acct.owner_key)), "you can only remove contact accounts");

      for( const auto& time_key_pair : acct.active_key_history )
      {
          keys.erase( address(time_key_pair.second) );
          address_to_account_wallet_record_index.erase( address(time_key_pair.second) );
      }
      remove_item( acct.wallet_record_index );

      keys.erase( address(acct.owner_key) );
      address_to_account_wallet_record_index.erase( address(acct.owner_key) );
      account_id_to_wallet_record_index.erase( acct.id );
      name_to_account_wallet_record_index.erase( account_name );
      accounts.erase( acct.wallet_record_index );
   }

   void wallet_db::rename_account(const public_key_type &old_account_key,
                                   const string& new_account_name )
   {
      /* Precondition: check that new_account doesn't exist in wallet and that old_account does
       */
      FC_ASSERT( is_open() );

      auto opt_old_acct = lookup_account( old_account_key );
      FC_ASSERT( opt_old_acct.valid() );
      auto acct = *opt_old_acct;
      auto old_name = acct.name;
      acct.name = new_account_name;
      name_to_account_wallet_record_index[acct.name] = acct.wallet_record_index;
      if( name_to_account_wallet_record_index[old_name] == acct.wallet_record_index )
        //Only remove the old name from the map if it pointed to the record we've just renamed
        name_to_account_wallet_record_index.erase( old_name );
      accounts[acct.wallet_record_index] = acct;
      address_to_account_wallet_record_index[address(acct.owner_key)] = acct.wallet_record_index;
      for( const auto& time_key_pair : acct.active_key_history )
          address_to_account_wallet_record_index[address(time_key_pair.second)] = acct.wallet_record_index;

      store_record( acct );
   }

   void wallet_db::store_transaction( wallet_transaction_record& trx_to_store, const bool sync )
   { try {
      if( trx_to_store.wallet_record_index == 0 )
         trx_to_store.wallet_record_index = new_wallet_record_index();
      store_record( trx_to_store, sync );

      transactions[ trx_to_store.record_id ] = trx_to_store;
   } FC_CAPTURE_AND_RETHROW( (trx_to_store) ) }

   void wallet_db::cache_account( const wallet_account_record& war, const bool sync )
   {
      accounts[war.wallet_record_index] = war;
      if( war.id != 0 )
      {
         account_id_to_wallet_record_index[war.id] = war.wallet_record_index;
         name_to_account_wallet_record_index[war.name] = war.wallet_record_index;
      }
      store_record( war, sync );
   }

   void wallet_db::remove_item( int32_t index )
   { try {
       try
       {
#ifndef BTS_TEST_NETWORK
           my->_records.remove( index, true ); // Sync
#else
           my->_records.remove( index );
#endif
       }
       catch( const fc::key_not_found_exception& )
       {
           wlog("wallet_db tried to remove nonexistent index: ${i}", ("i",index) );
       }
   } FC_CAPTURE_AND_RETHROW( (index) ) }

   bool wallet_db::validate_password( const fc::sha512& password )const
   {
      FC_ASSERT( wallet_master_key );
      return wallet_master_key->validate_password( password );
   }

   optional<extended_private_key> wallet_db::get_master_key( const fc::sha512& password    )const
   {
      FC_ASSERT( wallet_master_key );
      return wallet_master_key->decrypt_key( password );
   }

   void wallet_db::set_master_key( const extended_private_key& extended_key,
                                    const fc::sha512& new_password )
   {
      master_key key;
      key.encrypt_key(new_password,extended_key);
      auto key_record = wallet_master_key_record( key, -1 );
      store_record( key_record, true );
   }

   void wallet_db::change_password( const fc::sha512& old_password,
                                    const fc::sha512& new_password )
   { try {
      FC_ASSERT( wallet_master_key );
      auto old_key = get_master_key( old_password );
      FC_ASSERT( old_key, "unable to change password because old password was invalid" );
      set_master_key( *old_key, new_password );

      for( auto key : keys )
      {
         if( key.second.has_private_key() )
         {
            auto priv_key = key.second.decrypt_private_key( old_password );
            key.second.encrypt_private_key( new_password, priv_key );
            store_record( key.second, true );
         }
      }
   } FC_CAPTURE_AND_RETHROW() }

   void wallet_db::remove_balance( const balance_id_type& balance_id )
   {
      remove_item( balances[balance_id].wallet_record_index );
      balances.erase(balance_id);
   }

   void wallet_db::remove_transaction( const transaction_id_type& record_id )
   {
      const auto rec = lookup_transaction( record_id );
      if( !rec.valid() ) return;
      remove_item( rec->wallet_record_index );
      transactions.erase( record_id );
   }

} } // bts::wallet
