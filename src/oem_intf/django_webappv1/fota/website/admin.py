from django.contrib import admin
from .models import Fota_Firmware, Fota_Ecu, Fota_Vehicle, Fota_Fota, mqtt

admin.site.register(Fota_Firmware)
admin.site.register(Fota_Ecu)
admin.site.register(Fota_Vehicle)
admin.site.register(Fota_Fota)
admin.site.register(mqtt)
