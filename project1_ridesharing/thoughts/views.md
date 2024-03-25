# Views.py

- log in
  - don't have an account? Sign up
- create account
- log out

### Rider

- View ride status_rider (show only open and confirmed orders), the url should be /status_rider/

  - This view should be home page, with rough info (dest addr, order status, date and time) , and:
    - Add a button to say "request a ride" which links to /request_ride/
    - Each order item should:
      - have a button says "view detail" which links to corresponding detail page saying "/status_rider/\<that order id\>/detail/". This implementation will be later
      - provide a button saying "cancel" only when order status is open, pop up a deletion confirmation, and specifically:
        - if user is sharer of ride, simply delete him and his party of passengers from the order and redirect to same view page
        - if user is owner of ride, delete the whole order (and deleting any relative sharers and their parties if any) and redirect to same view page
      - If user is owner of the ride, and the order status is open, provide a button near "cancel", it should be a button saying "modify", which opens a new page "/status_rider/\<that order id\>/modification/", where he can modify below the chosen ride status these things (the original info should have been provided on this modification page), and there should be a button called "back" redirecting to /status_rider/
        - Modify destination address
        - Modify required arrival date
        - Modify required arrival time
        - Modify number of total passengers
        - (optional) Modify cehicle type
        - (optional) Modify any other special requests
        - Modify whether this ride can be shared or not. 
- View ride status in detail
  - for open rides: show current ride details (from the original request + any updates due to sharers joining the ride)
  - For confirmed rides: show all details for open rides, and also show driver and vehicle info details
  - add a return button to status page

- request a ride 

  - Specify destination address
  - Specify required arrival date
  - specify required arrival time
  - Specify number of total passengers
  - (optional) specify vehicle type
  - (optional) specify any other special requests
  - Specify whether this ride can be shared or not
- Modify a ride (this is a button in status view) (if is owner & available up until ride is confirmed a.k.a. is open)

  - Modify if self is joining the ride or not
  - if is ride owner  (by checking ride orderer's name):
    - Modify destination address
    - Modify required arrival date
    - Modify required arrival time
    - Modify number of total passengers
    - (optional) Modify cehicle type
    - (optional) Modify any other special requests
    - Modify whether this ride can be shared or not
      - Danger log: owner modifies this after a sharer joined in
- search for open ride_rider (should be blank at first with only 3 search indexes. once clicked on a searching index, show all the corresponding available open rides, each order item with a button says "join". Can tighten search by adding choices. If no order is found, say "No rides available")
  - provide a clickable search index by prompting user to specify destination address
  - provide a clickable search index by prompting user to specify arrival window (this is ride sharer's earliest and latese acceptable arrival date & time, should be the same date as order arrival date, and should be earlier than order arrival time)
  - provide a clickable search index by prompting user to specify number of  passengers in my party (should be less or equal than the remaining capacity of a certain car in the corresponding order)
  - button for Join ride, pop up confirmation, then redirect back to search page
  - button for going back to ride status, the button itself saying "view all rides"

### Driver

- search for open ride_driver (can tighten search by adding choices)
  - ONLY orders that match vehicle capacity (maximum number of passengers), vehicle type, and special request (if any, if either of all passengers required some) will be shown!
    - danger log: two riders add to same special request
  - based on destination address
  - based on required arrival date, then time
  - button: claim and start a ride
    - send confirmation email to passengers using API
    - turn the order status into confirmed
- view profile
  - View vehicle info (button: edit, then redirect to same page)
    - vehicle type
    - vehicle license plate number
    - (optional) special vehicle info
    - vehicle's maximum number of passengers
  - View first and last name (button: edit, then redirect to same page)
- View ride status_driver (show only her confirmed ones)
  - show ride owner username
  - show all ride sharer usernames
  - Show number of passengers in each party
  - Button: "edit", can mark the order as complete
  - This should be the home page for driver user. Add a button say "view profile" and a button say "search for ride"
- view_detail_driver