# 1. class Rider(models.Model)

##### fields

- username
- Password
- email address

- QuerySet of ride orders (her requested/joined orders)

## 1.2 subclass Driver(inherited from Rider)

##### fields

- is_active_driver
- First name
- Last name
- Vehicle type
- Vehicle license plate number
- (optional) special vehicle info
- Vehicle's maximum number of passengers
- QuerySet of ride orders (her confirmed orders)

# 2. class Order

##### fields

- Ride owner username (no repitition allowed, because we use name to check if a rider is ride owner)
- list of tuple of (riderusername, number of group people)
- order number
- Destination address
- Required arrival date
- Required arrival time
- Number of total passengers
- vehicle type
- (optional) special request
- If ride can be shared
- If ride is open
  - a ride is open from the time it's requested till it's confirmed
- If ride is confirmed
  - a ride becomes confirmed once a driver accepts the ride, and is in route



- Owned and joined orders will be handled through related_name in Order model    
- owned_orders provided by ForeignKey in Order    
- joined_orders provided by ManyToManyField in Order



- Confirmed orders for a driver will be handled through related_name in Order model  
- confirmed_orders provided by ForeignKey in Order