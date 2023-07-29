# Generated by Django 4.2.3 on 2023-07-28 15:01

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ("website", "0002_fota_firmware_firmware_size"),
    ]

    operations = [
        migrations.CreateModel(
            name="mqtt",
            fields=[
                (
                    "id",
                    models.BigAutoField(
                        auto_created=True,
                        primary_key=True,
                        serialize=False,
                        verbose_name="ID",
                    ),
                ),
                ("broker", models.CharField(max_length=255)),
                ("port", models.PositiveIntegerField()),
                ("client_id", models.CharField(max_length=255)),
                ("username", models.CharField(max_length=255)),
                ("password", models.CharField(max_length=255)),
                ("ka_topic", models.CharField(max_length=255)),
                ("pub_topic", models.CharField(max_length=255)),
                ("sub_topic", models.CharField(max_length=255)),
                ("out_msg", models.CharField(max_length=1024)),
                ("in_msg", models.CharField(max_length=1024)),
            ],
        ),
    ]