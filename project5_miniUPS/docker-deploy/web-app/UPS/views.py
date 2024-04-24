from django.shortcuts import render, redirect, get_object_or_404
from django.contrib.auth import login, authenticate, logout
from .forms import SignUpForm
from django.contrib.auth.forms import AuthenticationForm
from django.http import HttpResponse
from django.urls import reverse
from .models import Package, Delivery
from django.contrib.auth.decorators import login_required
from django.core.mail import send_mail


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
    try:
        package = Package.objects.get(tracking_number=tracking_number)
    except Package.DoesNotExist:
        return render(
            request,
            "UPS/frontpage.html",
            {"error": "The package you track does not exist."},
        )
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
        try:
            package = Package.objects.get(tracking_number=search_tracking_number)
        except Package.DoesNotExist:
            return render(
                request,
                "UPS/my_packages.html",
                {
                    "error": "The package you are tracking does not exist.",
                    "user": user,
                    "packages": Package.objects.filter(user=user),
                },
            )

        if package.user_id == user.user_id:
            return redirect(reverse("package_details", args=[search_tracking_number]))
        else:
            return redirect(
                reverse("package_basic_info", args=[search_tracking_number])
            )

    if status_filter in ["at warehouse", "loading", "delivering", "delivered"]:
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
        return HttpResponse(
            "You do not have permission to view this package.", status=403
        )

    deliveries = Delivery.objects.filter(package=package).order_by("go_warehouse_time")
    delivery_info = []

    for delivery in deliveries:
        truck_id = delivery.truck.truck_id

        if delivery.go_warehouse_time:
            delivery_info.append(
                f"Truck No. {truck_id} starts heading to the warehouse at {delivery.go_warehouse_time}."
            )

        if delivery.arrive_warehouse_time:
            delivery_info.append(
                f"Truck No. {truck_id} has arrived at the warehouse at {delivery.arrive_warehouse_time}."
            )

        if delivery.delivery_start_time:
            delivery_info.append(
                f"Truck No. {truck_id} is out for delivery at {delivery.delivery_start_time}."
            )

        if delivery.delivered_time:
            delivery_info.append(
                f"Truck No. {truck_id} delivered your package to {package.destination_address} at {delivery.delivered_time}."
            )

    if request.method == "POST" and package.status == "at warehouse":
        new_address = request.POST.get("new_address", "")
        if new_address:
            package.destination_address = new_address
            package.save()

            send_mail(
                "Package Destination Address Updated",
                f"Your package with tracking number {package.tracking_number} of contents: {package.whats_inside} has a new destination address: {new_address}. Contents: {package.whats_inside}.",
                "ridesharemyuber@gmail.com",
                [user.email],
                fail_silently=False,
            )

    context = {
        "tracking_number": package.tracking_number,
        "status": package.status,
        "destination_address": package.destination_address,
        "delivery_info": delivery_info,
    }

    return render(request, "UPS/package_details.html", context)


@login_required
def profile_view(request):
    user = request.user
    context = {
        "user_id": user.user_id,
        "username": user.username,
        "email": user.email,
    }

    if request.method == "POST":
        new_email = request.POST.get("new_email", "")
        if new_email:
            user.email = new_email
            user.save()

    return render(request, "UPS/profile.html", context)
