from django import forms
from django.forms.widgets import DateInput, TimeInput
from django.contrib.auth.forms import UserCreationForm, AuthenticationForm
from .models import Rider, Driver, Order
from django.core.validators import MinValueValidator


class RiderSignUpForm(UserCreationForm):
    email = forms.EmailField(max_length=255, help_text="Enter a valid email address")

    class Meta:
        model = Rider
        fields = ("username", "email", "password1", "password2")


class DriverSignUpForm(UserCreationForm):
    email = forms.EmailField(max_length=255, help_text="Enter a valid email address")
    first_name = forms.CharField(max_length=255)
    last_name = forms.CharField(max_length=255)
    vehicle_type = forms.ChoiceField(
        choices=[
            ("economy", "Economy"),
            ("premium", "Premium"),
            ("suv", "SUV"),
            ("electric", "Electric"),
            ("bike", "Bike and Scooter"),
        ],
        help_text="Select the vehicle type",
    )
    vehicle_license_plate_number = forms.CharField(max_length=255)
    special_vehicle_info = forms.ChoiceField(
        choices=[
            ("", "None"),
            ("pet-friendly", "Pet-friendly"),
            ("accessible", "Accessible"),
            ("child seat", "Child Seat"),
            ("female driver", "Female Driver"),
            ("extra luggage space", "Extra Luggage Space"),
        ],
        required=False,
        help_text="Select any special vehicle information",
        initial="",
    )
    vehicle_capacity = forms.IntegerField(
        validators=[MinValueValidator(1)],
        help_text="Enter a value greater than or equal to 1",
    )

    class Meta:
        model = Driver
        fields = (
            "username",
            "email",
            "password1",
            "password2",
            "first_name",
            "last_name",
            "vehicle_type",
            "vehicle_license_plate_number",
            "special_vehicle_info",
            "vehicle_capacity",
        )


class LoginForm(AuthenticationForm):
    username = forms.CharField(label="Username")


class RideRequestForm(forms.ModelForm):
    vehicle_type_choices = [
        ("", "Not specified"),
        ("economy", "Economy"),
        ("premium", "Premium"),
        ("suv", "SUV"),
        ("electric", "Electric"),
        ("bike", "Bike and Scooter"),
    ]
    special_request_choices = [
        ("", "None"),
        ("pet-friendly", "Pet-friendly"),
        ("accessible", "Accessible"),
        ("child seat", "Child Seat"),
        ("female driver", "Female Driver"),
        ("extra luggage space", "Extra Luggage Space"),
    ]
    vehicle_type = forms.ChoiceField(
        choices=vehicle_type_choices, required=False, initial=""
    )
    special_request = forms.ChoiceField(
        choices=special_request_choices, required=False, initial=""
    )
    total_passengers = forms.IntegerField(validators=[MinValueValidator(1)], initial=1)

    class Meta:
        model = Order
        fields = [
            "destination_address",
            "required_arrival_date",
            "required_arrival_time",
            "total_passengers",
            "vehicle_type",
            "special_request",
            "shareable",
        ]
        widgets = {
            "required_arrival_date": forms.DateInput(attrs={"type": "date"}),
            "required_arrival_time": forms.TimeInput(attrs={"type": "time"}),
        }


class SearchRidesForm(forms.Form):
    destination_address = forms.CharField(
        required=False,
        widget=forms.TextInput(attrs={"placeholder": "Destination Address"}),
    )
    arrival_date = forms.DateField(
        required=False, widget=forms.DateInput(attrs={"type": "date"})
    )
    arrival_time = forms.TimeField(
        required=False,
        widget=forms.TimeInput(attrs={"type": "time"}),
        help_text="Select a preferred time. Orders within ±15min of this time will be shown.",
    )


class DriverProfileForm(forms.ModelForm):
    VEHICLE_TYPE_CHOICES = [
        ("economy", "Economy"),
        ("premium", "Premium"),
        ("suv", "SUV"),
        ("electric", "Electric"),
        ("bike", "Bike and Scooter"),
    ]

    SPECIAL_VEHICLE_INFO_CHOICES = [
        ("", "None"),
        ("pet-friendly", "Pet-friendly"),
        ("accessible", "Accessible"),
        ("child seat", "Child Seat"),
        ("female driver", "Female Driver"),
        ("extra luggage space", "Extra Luggage Space"),
    ]

    vehicle_type = forms.ChoiceField(choices=VEHICLE_TYPE_CHOICES)
    special_vehicle_info = forms.ChoiceField(
        choices=SPECIAL_VEHICLE_INFO_CHOICES, required=False
    )
    vehicle_capacity = forms.IntegerField(validators=[MinValueValidator(1)])
    is_active_driver = forms.BooleanField(
        required=False,
        label="You would like to be a driver. \n(Note: if you choose no here and logout, you will become a rider permanently. In your current driver session, you can still complete orders, but you would be no longer able to confirm new orders)",
    )

    class Meta:
        model = Driver
        fields = [
            "first_name",
            "last_name",
            "vehicle_type",
            "vehicle_license_plate_number",
            "special_vehicle_info",
            "vehicle_capacity",
            "is_active_driver",
        ]
