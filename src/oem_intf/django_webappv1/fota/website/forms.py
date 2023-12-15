from django import forms
from .models import Fota_Fota


class FotaUpdateForm(forms.ModelForm):
    class Meta:
        model = Fota_Fota
        fields = [
            "firmware",
            "ecu",
            "vehicle",
            "availability_flag",
            "status",
            "started_at",
            "completed_at",
            "minutes_taken",
        ]
