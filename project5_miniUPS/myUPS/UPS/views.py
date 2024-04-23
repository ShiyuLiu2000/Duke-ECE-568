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