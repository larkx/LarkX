#include <bts/client/notifier.hpp>
#include <bts/blockchain/config.hpp>
#include <bts/blockchain/time.hpp>

#include <fc/network/gntp.hpp>

#include <sstream>

namespace bts { namespace client { 
  namespace detail 
  {
    class bts_gntp_notifier_impl
    {
    public:
      fc::gntp_notifier _notifier;
      fc::gntp_icon_ptr _bitshares_icon;
      std::string _bts_instance_identifier;

      bool     _shutting_down;

      uint32_t _last_reported_connection_count;
      uint32_t _connection_count_notification_threshold;
      fc::time_point _last_connection_count_notification_time;
      fc::microseconds _connection_count_notification_interval;
      fc::time_point _last_head_block_too_old_notification_time;
      fc::microseconds _head_block_too_old_notification_interval;
      uint32_t _missed_block_count_threshold;

      bts_gntp_notifier_impl(const std::string& host_to_notify = "127.0.0.1", uint16_t port = 23053,
                             const std::string& bts_instance_identifier = BTS_BLOCKCHAIN_NAME,
                             const fc::optional<std::string>& password = fc::optional<std::string>());
      void register_notification_types();
    };
    extern const unsigned char bitshares_icon_png[];
    extern const unsigned bitshares_icon_png_len;

    bts_gntp_notifier_impl::bts_gntp_notifier_impl(const std::string&  host_to_notify /* = "127.0.0.1" */, uint16_t port /* = 23053 */,
                                                   const std::string& bts_instance_identifier /* = "BitShares" */,
                                                   const fc::optional<std::string>& password /* = optional<std::string>() */) :
      _notifier(host_to_notify, port, password),
      _bitshares_icon(std::make_shared<fc::gntp_icon>((const char*)bitshares_icon_png, bitshares_icon_png_len)),
      _bts_instance_identifier(bts_instance_identifier),
      _shutting_down(false),
      _last_reported_connection_count(0),
      _connection_count_notification_threshold(5),
      _connection_count_notification_interval(fc::seconds(300)),
      _head_block_too_old_notification_interval(fc::seconds(300)),
      _missed_block_count_threshold(3)
    {
    }

    void bts_gntp_notifier_impl::register_notification_types()
    {
      fc::gntp_notification_type notification_type;

      notification_type.name = "connection_count_below_threshold";
      notification_type.display_name = "Connection Count Below Threshold";
      notification_type.enabled = true;
      notification_type.icon = _bitshares_icon;
      _notifier.add_notification_type(notification_type);

      notification_type.name = "client_exiting_unexpectedly";
      notification_type.display_name = "Client Exiting Unexpectedly";
      notification_type.enabled = true;
      notification_type.icon = _bitshares_icon;
      _notifier.add_notification_type(notification_type);

      notification_type.name = "head_block_too_old";
      notification_type.display_name = "Head Block is Too Old";
      notification_type.enabled = true;
      notification_type.icon = _bitshares_icon;
      _notifier.add_notification_type(notification_type);

      _notifier.register_notifications();
    }
  }

  bts_gntp_notifier::bts_gntp_notifier(const std::string& host_to_notify /* = "127.0.0.1" */, uint16_t port /* = 23053 */,
                                       const std::string& bts_instance_identifier /* = "BitShares" */,
                                       const fc::optional<std::string>& password /* = fc::optional<std::string>() */) : 
    my(new detail::bts_gntp_notifier_impl(host_to_notify, port, bts_instance_identifier, password))
  {
    my->_notifier.set_application_name(BTS_BLOCKCHAIN_NAME);
    my->_notifier.set_application_icon(my->_bitshares_icon);
    my->register_notification_types();
  }

  bts_gntp_notifier::~bts_gntp_notifier()
  {
  }

  void bts_gntp_notifier::client_is_shutting_down()
  {
    my->_shutting_down = true;
  }

  void bts_gntp_notifier::notify_connection_count_changed(uint32_t new_connection_count)
  {
    // notify any time we drop below the threshold, unless we've already done so recently
    // (to cut down on noise if we're oscillating around the threshold)
    if (new_connection_count >= my->_connection_count_notification_threshold)
      my->_last_reported_connection_count = new_connection_count;
    else if (new_connection_count < my->_connection_count_notification_threshold && 
             my->_last_reported_connection_count >= my->_connection_count_notification_threshold)
    {
      fc::time_point notification_time_cutoff = fc::time_point::now() - my->_connection_count_notification_interval;
      if (my->_last_connection_count_notification_time < notification_time_cutoff)
      {
        std::ostringstream message;
        message << my->_bts_instance_identifier << ": peer connection count dropped to " << new_connection_count << 
                   ", which is below the warning threshold of " << my->_connection_count_notification_threshold;
        my->_notifier.send_notification("connection_count_below_threshold", "Connection Count Below Threshold", message.str(), my->_bitshares_icon);
        my->_last_reported_connection_count = new_connection_count;
        my->_last_connection_count_notification_time = fc::time_point::now();
      }
    }
  }

  void bts_gntp_notifier::notify_client_exiting_unexpectedly()
  {
    std::ostringstream message;
    message << my->_bts_instance_identifier << ": client is exiting due to an unhandled exception";
    my->_notifier.send_notification("client_exiting_unexpectedly", "Client Exiting Unexpectedly", message.str(), my->_bitshares_icon);
  }

  void bts_gntp_notifier::notify_head_block_too_old(const fc::time_point_sec& head_block_age)
  {
    fc::time_point block_age_cutoff = fc::time_point::now() - fc::seconds(BTS_BLOCKCHAIN_BLOCK_INTERVAL_SEC * my->_missed_block_count_threshold);
    if (head_block_age < block_age_cutoff)
    {
      fc::time_point notification_time_cutoff = fc::time_point::now() - my->_head_block_too_old_notification_interval;
      if (my->_last_head_block_too_old_notification_time < notification_time_cutoff)
      {
        std::ostringstream message;
        uint32_t age_in_sec = bts::blockchain::now().sec_since_epoch() - head_block_age.sec_since_epoch();
        uint32_t missed_block_count = age_in_sec / BTS_BLOCKCHAIN_BLOCK_INTERVAL_SEC;
        message << my->_bts_instance_identifier << ": the last block on our blockchain is " << fc::get_approximate_relative_time_string(head_block_age, bts::blockchain::now(), " old") << 
                   ", meaning we've missed " << missed_block_count << " blocks";
        my->_notifier.send_notification("head_block_too_old", "Head Block is Too Old", message.str(), my->_bitshares_icon);
        my->_last_head_block_too_old_notification_time = fc::time_point::now();    
      }
    }
  }


  namespace detail
  {
    const unsigned char bitshares_icon_png[] = {
#include "icon.c"
    };
    const unsigned bitshares_icon_png_len = sizeof(bitshares_icon_png);

  }

} } // end namespace bts::client
