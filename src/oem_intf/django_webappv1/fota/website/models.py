from django.db import models


# TODO: Make the CMAC, Firemware automatically using python
class Fota_Firmware(models.Model):
    firmware_id = models.AutoField(primary_key=True)
    version = models.CharField(max_length=50)
    release_date = models.DateField()
    firmware = models.FileField(upload_to='firmwares/')
    is_active = models.BooleanField(default=False)
    firmware_cmac = models.CharField(max_length=1024)
    firmware_size = models.PositiveIntegerField(default=0)
    description = models.TextField()

    def __str__(self):
        return f"{self.firmware_id}"


class Fota_Ecu(models.Model):
    ecu_id = models.AutoField(primary_key=True)
    name = models.CharField(max_length=100, default=None)
    firmware = models.ForeignKey(Fota_Firmware, on_delete=models.CASCADE)
    description = models.TextField()

    def __str__(self):
        return f"{self.ecu_id}"


class Fota_Vehicle(models.Model):
    vehicle_id = models.AutoField(primary_key=True)
    model = models.CharField(max_length=100)
    year = models.PositiveIntegerField()
    ecu = models.ForeignKey(Fota_Ecu, on_delete=models.CASCADE)

    def __str__(self):
        return f"{self.vehicle_id}"


class Fota_Fota(models.Model):
    fota_id = models.AutoField(primary_key=True)
    firmware = models.ForeignKey(Fota_Firmware, on_delete=models.CASCADE)
    ecu = models.ForeignKey(Fota_Ecu, on_delete=models.CASCADE)
    vehicle = models.ForeignKey(Fota_Vehicle, on_delete=models.CASCADE)
    availability_flag = models.BooleanField(default=False)
    update_ready_flag = models.BooleanField(default=False)
    status_choices = [
        ('none', 'None'),
        ('pending', 'Pending'),
        ('in_progress', 'In Progress'),
        ('completed', 'Completed'),
        ('failed', 'Failed')
    ]
    status = models.CharField(
        max_length=20, choices=status_choices, default='none')
    started_at = models.DateTimeField(null=True, blank=True)
    completed_at = models.DateTimeField(null=True, blank=True)
    minutes_taken = models.PositiveIntegerField(default=0)

    def __str__(self):
        return f"FOTA: {self.vehicle} - {self.firmware} - {self.ecu} - Status: {self.status}"