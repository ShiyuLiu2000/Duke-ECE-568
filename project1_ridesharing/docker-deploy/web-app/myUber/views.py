from curses.ascii import HT
from django.shortcuts import render, redirect, get_object_or_404
from django.contrib.auth import login, authenticate, logout
from .forms import (
    RiderSignUpForm,
    DriverSignUpForm,
    LoginForm,
    RideRequestForm,
    SearchRidesForm,
    DriverProfileForm,
)
from django.contrib.auth.forms import UserCreationForm
from django.http import HttpResponse, HttpResponseRedirect
from django.db.models import Sum
from django.urls import is_valid_path, reverse
from django.contrib import messages
from .models import Rider, Driver, Order, RideParty
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

def rider_signup_view(request):
    if request.method == "POST":
        form = RiderSignUpForm(request.POST)
        if form.is_valid():
            user = form.save()
            login(request, user)
            return redirect("login")
    else:
        form = RiderSignUpForm()
    return render(request, "myUber/rider_signup.html", {"form": form})


def driver_signup_view(request):
    if request.method == "POST":
        form = DriverSignUpForm(request.POST)
        if form.is_valid():
            user = form.save(commit=False)
            user.is_active_driver = True
            user.save()
            login(request, user)
            return redirect("login")
    else:
        form = DriverSignUpForm()
    return render(request, "myUber/driver_signup.html", {"form": form})


def login_view(request):
    if request.method == "POST":
        form = LoginForm(data=request.POST)
        if form.is_valid():
            username = form.cleaned_data.get("username")
            password = form.cleaned_data.get("password")
            user = authenticate(request, username=username, password=password)
            if user is not None:
                login(request, user)
                # Check if the user is a driver
                if hasattr(user, "driver") and request.user.driver.is_active_driver:
                    return redirect("status_driver")
                else:
                    return redirect("status_rider")
            else:
                return HttpResponse("Invalid username or password")
    else:
        form = LoginForm()
    return render(request, "myUber/login.html", {"form": form})


def logout_view(request):
    logout(request)
    return redirect("login")


@login_required
def request_ride(request):
    if request.method == "POST":
        form = RideRequestForm(request.POST)
        if form.is_valid():
            ride_order = form.save(commit=False)
            arrival_datetime = datetime.datetime.combine(
                form.cleaned_data["required_arrival_date"],
                form.cleaned_data["required_arrival_time"],
                tzinfo=timezone.get_current_timezone(),
            )
            if arrival_datetime < timezone.now():
                messages.error(request, "You cannot request a ride in the past.")
                return render(request, "myUber/request_ride.html", {"form": form})
            ride_order.ride_owner = request.user
            ride_order.status = "open"
            ride_order.total_passengers = form.cleaned_data["total_passengers"]
            ride_order.save()
            ride_party = RideParty(
                ride=ride_order,
                rider=request.user,
                passengers=form.cleaned_data["total_passengers"],
            )
            ride_party.save()
            ride_order.riders.add(request.user)
            return redirect("status_rider")
    else:
        form = RideRequestForm()
    return render(request, "myUber/request_ride.html", {"form": form})


@login_required
def status_rider(request):
    orders = Order.objects.filter(
        status__in=["open", "confirmed"], riders=request.user
    ).order_by("required_arrival_date", "required_arrival_time")

    return render(request, "myUber/status_rider.html", {"orders": orders})


@login_required
def cancel_ride(request, order_id):
    order = Order.objects.get(id=order_id)
    if order.ride_owner == request.user:
        # if the user is the owner, delete the entire order
        order.delete()
    else:
        # if the user is a sharer, remove them and their party from the order
        ride_party = RideParty.objects.filter(ride=order, rider=request.user).first()
        if ride_party:
            order.total_passengers -= (
                ride_party.passengers
            )  # Deduct the number of passengers
            order.save(update_fields=["total_passengers"])
            order.riders.remove(request.user)
            ride_party.delete()
    return redirect("status_rider")


@login_required
def modify_ride(request, order_id):
    order = get_object_or_404(
        Order, id=order_id, ride_owner=request.user, status="open"
    )

    if request.method == "POST":
        form = RideRequestForm(request.POST, instance=order)
        if form.is_valid():
            modified_order = form.save(commit=False)
            modified_order.save()
            ride_party, created = RideParty.objects.update_or_create(
                ride=order,
                rider=request.user,
                defaults={"passengers": form.cleaned_data["total_passengers"]},
            )
            return redirect("status_rider")
    else:
        form = RideRequestForm(instance=order)

    return render(
        request, "myUber/modify_ride.html", {"form": form, "order_id": order_id}
    )


@login_required
def ride_detail(request, order_id):
    order = get_object_or_404(Order, id=order_id)

    context = {
        "order": order,
        "ride_parties": RideParty.objects.filter(ride=order),
    }

    if order.status == "confirmed" and order.confirmed_by:
        context["driver"] = order.confirmed_by

    return render(request, "myUber/ride_detail.html", context)


@login_required
def search_rides(request):
    form = SearchRidesForm(request.GET or None)
    results = Order.objects.none()
    orders = Order.objects.filter(riders=request.user).values_list("id", flat=True)

    if form.is_valid():
        destination_address = form.cleaned_data.get("destination_address")
        arrival_date = form.cleaned_data.get("arrival_date")
        arrival_time = form.cleaned_data.get("arrival_time")

        query = Order.objects.filter(status="open", shareable=True)

        if destination_address:
            query = query.filter(destination_address__icontains=destination_address)
        if arrival_date:
            query = query.filter(required_arrival_date=arrival_date)
        if arrival_time:
            start_window = (
                datetime.datetime.combine(datetime.date(1, 1, 1), arrival_time)
                - datetime.timedelta(minutes=15)
            ).time()
            end_window = (
                datetime.datetime.combine(datetime.date(1, 1, 1), arrival_time)
                + datetime.timedelta(minutes=15)
            ).time()

            orders = orders.filter(
                required_arrival_time__gte=start_window,
                required_arrival_time__lte=end_window,
            )

        query = query.exclude(id__in=orders)
        results = list(query)

    return render(
        request, "myUber/search_rides.html", {"form": form, "results": results}
    )


@login_required
@require_POST
def join_ride(request, order_id):
    order = get_object_or_404(Order, id=order_id, status="open", shareable=True)

    passengers = int(request.POST.get("passengers", 1))

    order.total_passengers += passengers
    order.save()

    RideParty.objects.update_or_create(
        ride=order, rider=request.user, defaults={"passengers": passengers}
    )

    order.riders.add(request.user)

    return redirect("status_rider")


@login_required
def status_driver(request):
    orders = Order.objects.filter(
        confirmed_by=request.user, status__in=["confirmed", "completed"]
    ).order_by("required_arrival_date", "required_arrival_time")
    return render(request, "myUber/status_driver.html", {"orders": orders})


@login_required
def order_detail(request, order_id):
    order = get_object_or_404(Order, id=order_id, confirmed_by=request.user.driver)
    ride_parties = order.parties.all()

    if request.method == "POST":
        if order.status != "completed":
            order.status = "completed"
            order.save()
            return redirect("status_driver")
        else:
            return HttpResponseNotAllowed(["GET"])

    return render(
        request,
        "myUber/order_detail.html",
        {"order": order, "ride_parties": ride_parties},
    )


@login_required
def driver_profile(request):
    is_editing = request.GET.get("edit") == "1"
    driver_instance = get_object_or_404(Driver, pk=request.user.pk)

    if request.method == "POST" and is_editing:
        form = DriverProfileForm(request.POST, instance=driver_instance)
        if form.is_valid():
            form.save()
            return redirect(reverse("driver_profile"))
    else:
        form = DriverProfileForm(instance=driver_instance)

    return render(
        request, "myUber/driver_profile.html", {"form": form, "is_editing": is_editing}
    )


@login_required
def search_ride_driver(request):
    driver = request.user.driver
    form = SearchRidesForm(request.GET or None)
    orders = Order.objects.filter(
        status="open", total_passengers__lte=driver.vehicle_capacity
    ).exclude(confirmed_by=driver)

    if form.is_valid():
        destination_address = form.cleaned_data.get("destination_address")
        arrival_date = form.cleaned_data.get("arrival_date")
        arrival_time = form.cleaned_data.get("arrival_time")

        if destination_address:
            orders = orders.filter(destination_address__icontains=destination_address)
        if arrival_date:
            orders = orders.filter(required_arrival_date=arrival_date)
        if arrival_time:
            start_window = (
                datetime.datetime.combine(datetime.date(1, 1, 1), arrival_time)
                - datetime.timedelta(minutes=15)
            ).time()
            end_window = (
                datetime.datetime.combine(datetime.date(1, 1, 1), arrival_time)
                + datetime.timedelta(minutes=15)
            ).time()

            orders = orders.filter(
                required_arrival_time__gte=start_window,
                required_arrival_time__lte=end_window,
            )

        vehicle_type_query = Q(vehicle_type="") | Q(vehicle_type=driver.vehicle_type)
        special_request_query = Q(special_request="") | Q(
            special_request=driver.special_vehicle_info
        )
        orders = orders.filter(
            vehicle_type_query
            & special_request_query
            & Q(total_passengers__lte=driver.vehicle_capacity)
        )

    return render(
        request, "myUber/search_ride_driver.html", {"form": form, "orders": orders}
    )


@login_required
def confirm_order(request, order_id):
    order = get_object_or_404(Order, id=order_id, status="open")
    if (
        request.user.is_authenticated
        and hasattr(request.user, "driver")
        and request.user.driver.is_active_driver
    ):
        order.status = "confirmed"
        order.confirmed_by = request.user.driver
        order.save()

        subject = 'Your ride has been confirmed'
        message = """Your ride with order number {} has been confirmed.\n\nThank you for using our service.\nBest regards,\nYour Ride Sharing Team""".format(order.order_number)
        from_email = 'your-email@example.com' 
        recipient_list = [order.ride_owner.email] + [rider.email for rider in order.riders.all()]  

        print("Sending email...")
        send_mail(subject, message, from_email, recipient_list, fail_silently=False)
        print("Email sent!")

        messages.success(request, 'Order confirmed successfully!')
    
        return redirect("status_driver")
    else:
        messages.error(request, "You do not have permission to confirm this order, please edit your driver status in your profile")
        return redirect("search_ride_driver")


@login_required
def complete_order(request, order_id):
    order = get_object_or_404(
        Order, id=order_id, confirmed_by=request.user.driver, status="confirmed"
    )
    if request.method == "POST":
        order.status = "completed"
        order.save()
    else:
        messages.error(request, "Invalid request method.")

    return HttpResponseRedirect(reverse("status_driver"))
