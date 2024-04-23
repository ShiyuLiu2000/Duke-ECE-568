from curses.ascii import HT
from django.shortcuts import render, redirect, get_object_or_404
from django.contrib.auth import login, authenticate, logout
from .forms import (
    SignUpForm,
)
from django.contrib.auth.forms import UserCreationForm, AuthenticationForm
from django.http import HttpResponse, HttpResponseRedirect
from django.db.models import Sum
from django.urls import is_valid_path, reverse
from django.contrib import messages
from .models import UserManager, User, Package, Truck, Delivery
from django.contrib.auth.decorators import login_required
from django.views.decorators.csrf import csrf_exempt
from django.http import HttpResponseNotAllowed
from django.db.models import Q
import datetime
from django.views.decorators.http import require_POST
from django.utils import timezone
from django.core.mail import send_mail
from django.conf import settings
import os


def frontpage(request):
    return render(request, "UPS/frontpage.html")


def signup_view(request):
    if request.method == "POST":
        form = SignUpForm(request.POST)
        if form.is_valid():
            user = form.save()
            login(request, user)
            return redirect("login")
    else:
        form = SignUpForm()
    return render(request, "UPS/signup.html", {"form": form})


def login_view(request):
    if request.method == "POST":
        form = AuthenticationForm(request, data=request.POST)
        if form.is_valid():
            username = form.cleaned_data.get("username")
            password = form.cleaned_data.get("password")
            user = authenticate(request, username=username, password=password)
            if user:
                login(request, user)
                return redirect("myPackages")
            else:
                return HttpResponse("Invalid username or password")
    else:
        form = AuthenticationForm()
    return render(request, "UPS/login.html", {"form": form})


def track_basic_package(request):
    tracking_number = request.GET.get("tracking_number")

    if not tracking_number:
        return HttpResponse("Tracking number is required.", status=400)

    return redirect(reverse("package_basic_info", args=[tracking_number]))


def package_basic_info(request, tracking_number):
    package = get_object_or_404(Package, tracking_number=tracking_number)
    context = {
        "tracking_number": package.tracking_number,
        "status": package.status,
    }

    return render(request, "UPS/package_basic_info.html", context)


def logout_view(request):
    logout(request)
    return redirect("frontpage")

@login_required
def my_packages(request):
    user = request.user
    search_tracking_number = request.GET.get("tracking_number", "")
    status_filter = request.GET.get("status", "")

    if search_tracking_number:
        package = get_object_or_404(Package, tracking_number=search_tracking_number)

        if package.user_id == user.user_id:
            return redirect(reverse("package_details", args=[search_tracking_number]))
        else:
            return redirect(reverse("package_basic_info", args=[search_tracking_number]))

    if status_filter:
        packages = Package.objects.filter(user=user, status=status_filter)
    else:
        packages = Package.objects.filter(user=user)

    context = {
        "user": user,
        "packages": packages,
    }

    return render(request, "UPS/my_packages.html", context)


@login_required
def package_details(request, tracking_number):
    user = request.user
    package = get_object_or_404(Package, tracking_number=tracking_number)

    if package.user_id != user.user_id:
        return HttpResponse("You do not have permission to view this package.", status=403)

    context = {
        "tracking_number": package.tracking_number,
        "status": package.status,
        "destination_address": package.destination_address,
    }

    # truck and delivery information
    deliveries = Delivery.objects.filter(package=package).order_by("goWarehouse_time")
    delivery_info = []

    for delivery in deliveries:
        truck_id = delivery.truck.truck_id

        if delivery.goWarehouse_time:
            delivery_info.append(
                f"Truck No. {truck_id} starts heading to the warehouse at {delivery.goWarehouse_time}."
            )

        if delivery.arriveWarehouse_time:
            delivery_info.append(
                f"Truck No. {truck_id} has arrived at the warehouse at {delivery.arriveWarehouse_time}."
            )

        if delivery.delivery_start_time:
            delivery_info.append(
                f"Truck No. {truck_id} is out for delivery at {delivery.delivery_start_time}."
            )

        if delivery.delivered_time:
            delivery_info.append(
                f"Truck No. {truck_id} delivered your package to {package.destination_address} at {delivery.delivered_time}."
            )

    context["delivery_info"] = delivery_info

    # address editing
    if request.method == "POST" and package.status == "at warehouse":
        new_address = request.POST.get("new_address", "")
        if new_address:
            package.destination_address = new_address
            package.save()

    return render(request, "UPS/package_details.html", context)