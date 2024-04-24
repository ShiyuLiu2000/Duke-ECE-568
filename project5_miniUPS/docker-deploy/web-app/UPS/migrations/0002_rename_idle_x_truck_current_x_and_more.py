# Generated by Django 4.2.10 on 2024-04-23 20:12

from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('UPS', '0001_initial'),
    ]

    operations = [
        migrations.RenameField(
            model_name='truck',
            old_name='idle_x',
            new_name='current_x',
        ),
        migrations.RenameField(
            model_name='truck',
            old_name='idle_y',
            new_name='current_y',
        ),
        migrations.AddField(
            model_name='package',
            name='Amazon_id',
            field=models.CharField(blank=True, max_length=255, null=True),
        ),
        migrations.AddField(
            model_name='package',
            name='desAddr_x',
            field=models.IntegerField(blank=True, null=True),
        ),
        migrations.AddField(
            model_name='package',
            name='desAddr_y',
            field=models.IntegerField(blank=True, null=True),
        ),
        migrations.AddField(
            model_name='package',
            name='truck',
            field=models.ForeignKey(blank=True, null=True, on_delete=django.db.models.deletion.SET_NULL, to='UPS.truck'),
        ),
        migrations.AddField(
            model_name='package',
            name='whid',
            field=models.IntegerField(blank=True, null=True),
        ),
        migrations.AlterField(
            model_name='package',
            name='status',
            field=models.CharField(choices=[('at warehouse', 'At Warehouse'), ('loading', 'Loading'), ('delivering', 'Delivering'), ('delivered', 'Delivered')], max_length=50),
        ),
    ]
