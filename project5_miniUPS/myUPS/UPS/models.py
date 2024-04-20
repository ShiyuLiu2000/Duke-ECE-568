from django.db import models
from django.contrib.auth.models import AbstractBaseUser, BaseUserManager


class UserManager(BaseUserManager):
    def create_user(self, username, email, password=None):
        if not email:
            raise ValueError("Users must have an email address")
        if not username:
            raise ValueError("Users must have a username")

        user = self.model(
            username=username,
            email=self.normalize_email(email),
        )

        user.set_password(password)
        user.save(using=self._db)
        return user

    def create_superuser(self, username, email, password):
        user = self.create_user(
            username,
            email,
            password=password,
        )
        user.is_admin = True
        user.save(using=self._db)
        return user


class User(AbstractBaseUser):
    user_id = models.AutoField(primary_key=True)
    username = models.CharField(max_length=255, unique=True)
    email = models.EmailField(verbose_name="email address", max_length=255, unique=True)
    password = models.CharField(max_length=255)

    objects = UserManager()

    USERNAME_FIELD = "username"
    REQUIRED_FIELDS = ["email", "password"]

    def __str__(self):
        return self.username


class Package(models.Model):
    package_id = models.AutoField(primary_key=True)
    user = models.ForeignKey(User, on_delete=models.SET_NULL, null=True, blank=True)
    tracking_number = models.CharField(max_length=255, unique=True)
    status = models.CharField(
        max_length=50,
        choices=[
            ("at warehouse", "At Warehouse"),
            ("delivering", "Delivering"),
            ("delivered", "Delivered"),
        ],
    )
    destination_address = models.CharField(max_length=255)

    def __str__(self):
        return f"{self.tracking_number} - {self.status}"


class Truck(models.Model):
    truck_id = models.AutoField(primary_key=True)
    status = models.CharField(
        max_length=50,
        choices=[
            ("idle", "Idle"),
            ("go to warehouse", "Go to Warehouse"),
            ("arrive at warehouse", "Arrive at Warehouse"),
            ("delivering", "Delivering"),
        ],
    )
    idle_x = models.IntegerField()
    idle_y = models.IntegerField()

    def __str__(self):
        return f"Truck {self.truck_id} - {self.status}"


class Delivery(models.Model):
    delivery_id = models.AutoField(primary_key=True)
    truck = models.ForeignKey(Truck, on_delete=models.CASCADE)
    package = models.ForeignKey(Package, on_delete=models.CASCADE)
    pickup_location = models.CharField(max_length=255)
    goWarehouse_time = models.DateTimeField(null=True, blank=True)
    arriveWarehouse_time = models.DateTimeField(null=True, blank=True)
    delivery_start_time = models.DateTimeField(null=True, blank=True)
    delivered_time = models.DateTimeField(null=True, blank=True)

    def __str__(self):
        return f"Delivery {self.delivery_id} for Package {self.package.tracking_number}"
