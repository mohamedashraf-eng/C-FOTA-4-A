from django.urls import path
from . import views

urlpatterns = [
    path("", views.home, name="home"),
    path("logout/", views.logout_user, name="logout"),
    path("live_dashboard/", views.live_dashboard, name="live dashboard"),
    path("database/", views.fota_database, name="database"),
    path(
        "database/firmware_database/<int:pk>",
        views.firmware_database,
        name="firmware_database",
    ),
    path(
        "database/firmware_database/firmware_view",
        views.firmware_view,
        name="firmware_view",
    ),
    path("database/ecu_database/<int:pk>", views.ecu_database, name="ecu_database"),
    path(
        "database/vehicle_database/<int:pk>",
        views.vehicle_database,
        name="vehicle_database",
    ),
    path("database/fota_database/<int:pk>", views.fota_by_id, name="fota_by_id"),
    path(
        "update_fota_record/<int:pk>/",
        views.update_fota_record,
        name="update_fota_record",
    ),
    path("communications/", views.comm_intf, name="comm_intf"),
]
