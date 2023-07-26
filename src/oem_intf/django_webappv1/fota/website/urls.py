from django.urls import path
from . import views

urlpatterns = [
    path("", views.home, name='home'),
    path("logout/", views.logout_user, name='logout'),
    path("live_dashboard/", views.live_dashboard, name='live dashboard'),
    path("database/", views.fota_database, name='database'),
    path("database/firmware_database/<int:pk>",
         views.firmware_database, name='firmware_database'),
    path("database/ecu_database/<int:pk>",
         views.ecu_database, name='ecu_database'),
    path("database/vehicle_database/<int:pk>",
         views.vehicle_database, name='vehicle_database'),
    path("database/update_firmware_record",
         views.update_firmware_record, name='update_firmware_record'),
]