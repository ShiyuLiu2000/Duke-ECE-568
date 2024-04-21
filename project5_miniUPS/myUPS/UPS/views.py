from django.shortcuts import render, redirect
from django.urls import reverse
from .forms import SignUpForm
from django.contrib.auth import authenticate, login
from django.contrib.auth.forms import AuthenticationForm


def frontpage(request):
    return render(request, "UPS/frontpage.html")


def signup_view(request):
    if request.method == "POST":
        form = SignUpForm(request.POST)
        if form.is_valid():
            form.save()
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
            if user is not None:
                login(request, user)
                return redirect("myPackages")
            else:
                form.add_error(None, "Invalid username or password.")
    else:
        form = AuthenticationForm()

    return render(request, "UPS/login.html", {"form": form})
