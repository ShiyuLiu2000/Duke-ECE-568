from django.db import models
from django.contrib.auth.models import AbstractBaseUser, BaseUserManager


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
        user.set_password(password)
        user.save(using=self._db)
        return user


class User(AbstractBaseUser):
    user_id = models.AutoField(primary_key=True)
    username = models.CharField(max_length=255, unique=True)
    email = models.EmailField(verbose_name="email address", max_length=255, unique=True)
    password = models.CharField(max_length=128)
    objects = UserManager()
    USERNAME_FIELD = "username"
    REQUIRED_FIELDS = ["email", "password"]

    def __str__(self):
        return self.username


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
    current_x = models.IntegerField()
    current_y = models.IntegerField()

    def __str__(self):
        return f"Truck {self.truck_id} - {self.status}"


class Package(models.Model):
    user = models.ForeignKey(User, on_delete=models.SET_NULL, null=True, blank=True)
    tracking_number = models.CharField(max_length=255, unique=True, primary_key=True)
    status = models.CharField(
        max_length=50,
        choices=[
            ("at warehouse", "At Warehouse"),
            ("loading", "Loading"),
            ("delivering", "Delivering"),
            ("delivered", "Delivered"),
        ],
    )
    destination_address = models.CharField(max_length=255)
    whats_inside = models.CharField(max_length=255)

    whid = models.IntegerField(null=True, blank=True)
    amazon_id = models.CharField(max_length=255, null=True, blank=True)
    dest_addr_x = models.IntegerField(null=True, blank=True)
    dest_addr_y = models.IntegerField(null=True, blank=True)
    truck = models.ForeignKey(Truck, on_delete=models.SET_NULL, null=True, blank=True)

    def __str__(self):
        return f"{self.tracking_number} - {self.status}"


class Delivery(models.Model):
    delivery_id = models.AutoField(primary_key=True)
    truck = models.ForeignKey(Truck, on_delete=models.CASCADE)
    package = models.ForeignKey(Package, on_delete=models.CASCADE)
    pickup_location = models.CharField(max_length=255)
    go_warehouse_time = models.DateTimeField(null=True, blank=True)
    arrive_warehouse_time = models.DateTimeField(null=True, blank=True)
    delivery_start_time = models.DateTimeField(null=True, blank=True)
    delivered_time = models.DateTimeField(null=True, blank=True)

    def __str__(self):
        return f"Delivery {self.delivery_id} for Package {self.package.tracking_number}"
