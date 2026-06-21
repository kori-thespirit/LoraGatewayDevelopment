#ifndef NET_EG_DEFINE_H
#define NET_EG_DEFINE_H

typedef enum{
  NET_WIFI_IS_CONNECTED = 0,
  NET_ETH_IS_CONNECTED,
  NET_MQTT_IS_CONNECTED,
  NET_MQTT_IS_SUBSCRIBED,
  NET_EG_BIT_MAX,
}e_net_eg_bit;

#define BIT_TO_VALUE(_BIT_) (1UL << (_BIT_))
#define CHECK_BIT(_VALUE_, _BIT_) ((_VALUE_) & (_BIT_) ? 1 : 0)
#endif // NET_EG_DEFINE_H
