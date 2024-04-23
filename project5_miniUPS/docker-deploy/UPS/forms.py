from django import forms
from django.contrib.auth.forms import UserCreationForm, AuthenticationForm
from .models import User


class SignUpForm(UserCreationForm):
    email = forms.EmailField(max_length=255, help_text="Enter a valid email address")

    class Meta:
        model = User
        fields = ("username", "email", "password1", "password2")

