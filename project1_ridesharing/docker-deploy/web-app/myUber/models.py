from django.db import models
from django.contrib.auth.models import AbstractBaseUser, BaseUserManager
from django.utils import timezone
import uuid


class UserManager(BaseUserManager):
    def create_user(self, email, username, password=None):
        if not email:
            raise ValueError("Please enter email address")
        if not username:
            raise ValueError("Please enter username")

        user = self.model(
            email=self.normalize_email(email),
            username=username,
        )

        user.set_password(password)
        user.save(using=self._db)
        return user

    def create_superuser(self, email, username, password=None):
        user = self.create_user(
            email,
            password=password,
            username=username,
        )
        user.is_admin = True
        user.save(using=self._db)
        return user


class Rider(AbstractBaseUser):
    username = models.CharField(max_length=255, unique=True)
    email = models.EmailField(verbose_name="email address", max_length=255, unique=True)
    USERNAME_FIELD = "username"
    REQUIRED_FIELDS = ["email", "password"]
    password = models.CharField(max_length=128, null=True)
    objects = UserManager()

    def __str__(self):
        return self.username


class Driver(Rider):
    is_active_driver = models.BooleanField(default=False)
    first_name = models.CharField(max_length=255)
    last_name = models.CharField(max_length=255)
    vehicle_type = models.CharField(max_length=255)
    vehicle_license_plate_number = models.CharField(max_length=255)
    special_vehicle_info = models.TextField(blank=True, null=True)
    vehicle_capacity = models.IntegerField()

    def __str__(self):
        return f"{self.first_name} {self.last_name} - {self.vehicle_type}"


class Order(models.Model):
    # related_name provides a reversed fetching relationship
    ride_owner = models.ForeignKey(
        Rider, on_delete=models.CASCADE, related_name="owned_orders"
    )
    riders = models.ManyToManyField(Rider, related_name="joined_orders")
    confirmed_by = models.ForeignKey(
        Driver,
        on_delete=models.SET_NULL,
        null=True,
        blank=True,
        related_name="confirmed_orders",
    )
    order_number = models.UUIDField(default=uuid.uuid4, editable=False, unique=True)
    destination_address = models.CharField(max_length=255)
    required_arrival_date = models.DateField()
    required_arrival_time = models.TimeField()
    total_passengers = models.IntegerField()
    vehicle_type = models.CharField(max_length=255, blank=True, null=True)
    special_request = models.TextField(blank=True, null=True)
    shareable = models.BooleanField(default=False)
    # is_open = models.BooleanField(default=True)
    # is_confirmed = models.BooleanField(default=False)
    STATUS_CHOICES = (
        ("open", "Open"),
        ("confirmed", "Confirmed"),
        ("completed", "Completed"),
    )
    status = models.CharField(max_length=10, choices=STATUS_CHOICES, default="open")


class RideParty(models.Model):
    ride = models.ForeignKey(Order, on_delete=models.CASCADE, related_name="parties")
    rider = models.ForeignKey(Rider, on_delete=models.CASCADE)
    passengers = models.IntegerField(
        default=1, help_text="Number of passengers in the rider's party"
    )

    def __str__(self):
        return f"{self.ride.order_number} - {self.rider.username} - Passengers: {self.passengers}"
