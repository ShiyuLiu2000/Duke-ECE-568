from django.urls import path
from . import views
from django.contrib.auth.views import LogoutView

urlpatterns = [
    path("login/", views.login_view, name="login"),
    path("signup/", views.rider_signup_view, name="signup"),
    path("signup/driver/", views.driver_signup_view, name="driver_signup"),
    path("request_ride/", views.request_ride, name="request_ride"),
    path("status_rider/", views.status_rider, name="status_rider"),
    path("status_rider/<int:order_id>/detail/", views.ride_detail, name="ride_detail"),
    path("status_rider/<int:order_id>/cancel/", views.cancel_ride, name="cancel_ride"),
    path(
        "status_rider/<int:order_id>/modification/",
        views.modify_ride,
        name="modify_ride",
    ),
    path("search_rides/", views.search_rides, name="search_rides"),
    path("join_ride/<int:order_id>/", views.join_ride, name="join_ride"),
    path("status_driver/", views.status_driver, name="status_driver"),
    path(
        "status_driver/<int:order_id>/detail/", views.order_detail, name="order_detail"
    ),
    path("driver_profile/", views.driver_profile, name="driver_profile"),
    path("search_ride_driver/", views.search_ride_driver, name="search_ride_driver"),
    path("confirm_order/<int:order_id>/", views.confirm_order, name="confirm_order"),
    path("complete_order/<int:order_id>/", views.complete_order, name="complete_order"),
    path('logout/', LogoutView.as_view(next_page='login'), name='logout'),
]
