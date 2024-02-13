# Generated by Django 4.2.9 on 2024-02-04 10:33

from django.conf import settings
from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('myUber', '0003_alter_rider_password'),
    ]

    operations = [
        migrations.RemoveField(
            model_name='order',
            name='is_confirmed',
        ),
        migrations.RemoveField(
            model_name='order',
            name='is_open',
        ),
        migrations.AddField(
            model_name='order',
            name='status',
            field=models.CharField(choices=[('open', 'Open'), ('confirmed', 'Confirmed'), ('completed', 'Completed')], default='open', max_length=10),
        ),
        migrations.CreateModel(
            name='RideParty',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('passengers', models.IntegerField(default=1, help_text="Number of passengers in the rider's party")),
                ('ride', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name='parties', to='myUber.order')),
                ('rider', models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to=settings.AUTH_USER_MODEL)),
            ],
        ),
    ]
