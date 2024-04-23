from django.urls import path
from . import views
from django.contrib.auth.views import LogoutView

urlpatterns = [
    path("frontpage/", views.frontpage, name="frontpage"),
    path("login/", views.login_view, name="login"),
    path("signup/", views.signup_view, name="signup"),
    path("track/", views.track_basic_package, name="track_basic_package"),
    path(
        "<int:tracking_number>/basic_info/",
        views.package_basic_info,
        name="package_basic_info",
    ),
    path("logout/", LogoutView.as_view(next_page="frontpage"), name="logout"),
]
