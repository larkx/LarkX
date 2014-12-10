#include <bts/blockchain/account_operations.hpp>
#include <bts/blockchain/chain_interface.hpp>
#include <bts/blockchain/exceptions.hpp>
#include <bts/blockchain/pts_config.hpp>
#include <bts/blockchain/transaction_evaluation_state.hpp>
#include <fc/time.hpp>

namespace bts { namespace blockchain {

   bool register_account_operation::is_delegate()const
   {
       return delegate_pay_rate <= 100;
   }

   void register_account_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      auto now = eval_state._current_state->now();

      if( !eval_state._current_state->is_valid_account_name( this->name ) )
         FC_CAPTURE_AND_THROW( invalid_account_name, (name) );

      FC_ASSERT( this->name.size() >= PTS_INITIAL_MIN_LENGTH,
                 "Account names shorter than 5 characters cannot be registered at this time.");

      eval_state.required_fees += asset( eval_state._current_state->get_account_registration_fee(),
                                         0 );

      FC_ASSERT( banned_names.find( this->name ) == banned_names.end(),
                 "This account name is a reserved word. Operation failed.");

      auto current_account = eval_state._current_state->get_account_record( this->name );
      if( current_account ) FC_CAPTURE_AND_THROW( account_already_registered, (name) );

      if( eval_state._current_state->get_parent_account_name( this->name ).valid() )
      {
          if( !eval_state.any_parent_has_signed( this->name ) )
              FC_CAPTURE_AND_THROW( missing_parent_account_signature, (*this) );
      }

      auto account_with_same_key = eval_state._current_state->get_account_record( address(this->active_key) );
      if( account_with_same_key.valid() )
         FC_CAPTURE_AND_THROW( account_key_in_use, (active_key)(account_with_same_key) );

      account_record new_record;
      new_record.id                = eval_state._current_state->new_account_id();
      new_record.name              = this->name;
      new_record.public_data       = this->public_data;
      new_record.owner_key         = this->owner_key;
      new_record.registration_date = now;
      new_record.last_update       = now;

      new_record.set_active_key( now, this->active_key );
      if( this->is_delegate() )
      {
          new_record.delegate_info = delegate_stats();
          new_record.delegate_info->pay_rate = this->delegate_pay_rate;
          const auto max_reg_fee = eval_state._current_state->get_delegate_registration_fee();
          eval_state.required_fees += asset( (max_reg_fee * delegate_pay_rate) / 100, 0 );
      }
      new_record.meta_data = this->meta_data;

      eval_state._current_state->store_account_record( new_record );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   bool update_account_operation::is_delegate()const
   {
       return delegate_pay_rate <= 100;
   }

   void update_account_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      auto current_record = eval_state._current_state->get_account_record( this->account_id );
      if( !current_record ) FC_CAPTURE_AND_THROW( unknown_account_id, (account_id) );

      if( current_record->is_retracted() )
          FC_CAPTURE_AND_THROW( account_retracted, (current_record) );

      // If updating active key
      if( this->active_key.valid() && *this->active_key != current_record->active_key() )
      {
          const oaccount_record account_with_same_key = eval_state._current_state->get_account_record( *this->active_key );
          if( account_with_same_key.valid() )
              FC_CAPTURE_AND_THROW( account_key_in_use, (*this->active_key)(account_with_same_key) );

          if( !eval_state.check_signature( current_record->owner_key ) && !eval_state.any_parent_has_signed( current_record->name ) )
              FC_CAPTURE_AND_THROW( missing_signature, (*this) );

          current_record->set_active_key( eval_state._current_state->now(), *this->active_key );
      }
      else
      {
          if( !eval_state.account_or_any_parent_has_signed( *current_record ) )
              FC_CAPTURE_AND_THROW( missing_signature, (*this) );
      }

      if( this->public_data.valid() )
      {
         current_record->public_data  = *this->public_data;
      }

      if( current_record->is_delegate() )
      {
          // Delegates accounts cannot revert to a normal account
          FC_ASSERT( this->is_delegate() );
      }

      if( this->is_delegate() )
      {
         if( current_record->is_delegate() )
         {
            // Delegates cannot increase their pay rate
            FC_ASSERT( current_record->delegate_info->pay_rate >= this->delegate_pay_rate );
            current_record->delegate_info->pay_rate = this->delegate_pay_rate;
         }
         else
         {
            current_record->delegate_info = delegate_stats();
            current_record->delegate_info->pay_rate = this->delegate_pay_rate;
            const auto max_reg_fee = eval_state._current_state->get_delegate_registration_fee();
            eval_state.required_fees += asset( (max_reg_fee * this->delegate_pay_rate) / 100, 0 );
         }
      }

      current_record->last_update   = eval_state._current_state->now();

      if( this->active_key.valid() )
      {
         current_record->set_active_key( eval_state._current_state->now(), *this->active_key );
         auto account_with_same_key = eval_state._current_state->get_account_record( address(*this->active_key) );
         if( account_with_same_key )
            FC_CAPTURE_AND_THROW( account_key_in_use, (active_key)(account_with_same_key) );
      }

      eval_state._current_state->store_account_record( *current_record );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void withdraw_pay_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      if( this->amount <= 0 ) FC_CAPTURE_AND_THROW( negative_withdraw, (amount) );

      auto pay_to_account_id = abs(this->account_id);
      auto pay_to_account = eval_state._current_state->get_account_record( pay_to_account_id );
      if( !pay_to_account ) FC_CAPTURE_AND_THROW( unknown_account_id, (pay_to_account_id) );
      if( pay_to_account->is_retracted() ) FC_CAPTURE_AND_THROW( account_retracted, (pay_to_account) );
      if( !pay_to_account->is_delegate() ) FC_CAPTURE_AND_THROW( not_a_delegate, (pay_to_account) );

      auto active_key = pay_to_account->active_key();
      if( !eval_state.check_signature( active_key ) )
         FC_CAPTURE_AND_THROW( missing_signature, (active_key) );

      eval_state.net_delegate_votes[ pay_to_account_id ].votes_for -= this->amount;

      if( pay_to_account->delegate_info->pay_balance < this->amount )
         FC_CAPTURE_AND_THROW( insufficient_funds, (pay_to_account)(amount) );

      pay_to_account->delegate_info->pay_balance -= this->amount;

      eval_state._current_state->store_account_record( *pay_to_account );
      eval_state.add_balance( asset(this->amount, 0) );

   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void link_account_operation::evaluate( transaction_evaluation_state& eval_state )
   { try {
      FC_ASSERT( !"Link account operation is not enabled yet!" );

      auto source_account_rec = eval_state._current_state->get_account_record( source_account );
      if( !source_account_rec.valid() )
          FC_CAPTURE_AND_THROW( unknown_account_id, (*this) );

      if( source_account_rec->is_retracted() )
          FC_CAPTURE_AND_THROW( account_retracted, (*this) );

      auto destination_account_rec = eval_state._current_state->get_account_record( destination_account );
      if( !destination_account_rec.valid() )
          FC_CAPTURE_AND_THROW( unknown_account_id, (*this) );

      if( destination_account_rec->is_retracted() )
          FC_CAPTURE_AND_THROW( account_retracted, (*this) );

      if( !eval_state.account_or_any_parent_has_signed( *source_account_rec ) )
          FC_CAPTURE_AND_THROW( missing_signature, (*this) );

      // STORE LINK...
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

} } // bts::blockchain
