# Generated by Django 4.2.10 on 2024-04-21 23:14

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('UPS', '0002_alter_user_password'),
    ]

    operations = [
        migrations.AlterField(
            model_name='user',
            name='password',
            field=models.CharField(default='default_password', max_length=128),
            preserve_default=False,
        ),
    ]
