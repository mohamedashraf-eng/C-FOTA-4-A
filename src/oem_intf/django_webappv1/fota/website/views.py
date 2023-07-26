from django.shortcuts import render, redirect
from django.contrib.auth import authenticate, login, logout
from django.contrib import messages
#
#
from .models import Fota_Firmware, Fota_Ecu, Fota_Vehicle, Fota_Fota
from .forms import FotaUpdateForm


def home(request):
    # Check if logging in
    if request.method == "POST":
        username = request.POST['username']
        password = request.POST['password']
        # Authenticate
        user = authenticate(request, username=username, password=password)
        if user is not None:
            login(request, user)
            messages.success(request, "Logged in successfully")
            return redirect('home')
        else:
            messages.success(
                request, "There was an error loggin in. Please try again...")
            return redirect('home')
    else:
        return render(request, 'home.html', {})


def logout_user(request):
    logout(request)
    messages.success(request, "logged out successfully ")
    return redirect('home')


def live_dashboard(request):
    return render(request, 'live_dashboard.html', {})


def fota_database(request):
    if request.user.is_authenticated:
        fota_fota = Fota_Fota.objects.all()
        return render(request, 'database.html', {'fotas': fota_fota})
    else:
        messages.success(
            request, "You must be logged in to view the database")
        return redirect('home')


def fota_by_id(request, pk):
    if request.user.is_authenticated:
        fota_fota = Fota_Fota.objects.get(fota_id=pk)
        return render(request, 'fota.html', {'fotas': fota_fota})
    else:
        messages.success(
            request, "You must be logged in to view the database")
        return redirect('home')


def update_fota_record(request, pk):
    if request.user.is_authenticated:
        current_fota_record = Fota_Fota.objects.get(fota_id=pk)
        form = FotaUpdateForm(request.POST or None,
                              instance=current_fota_record)
        if form.is_valid():
            form.save()
            messages.success(
                request, "Record has been updated successfully")
            return redirect('database')
        return render(request, 'update_fota_record.html', {'form': form, 'fota_id': current_fota_record.fota_id})
    else:
        messages.success(
            request, "You must be logged in")
        return redirect('home')


def firmware_database(request, pk):
    if request.user.is_authenticated:
        fota_firmware = Fota_Firmware.objects.get(firmware_id=pk)
        return render(request, 'firmwares.html', {'firmwares': fota_firmware})
    else:
        messages.success(
            request, "You must be logged in to view the database")
        return redirect('home')


def ecu_database(request, pk):
    if request.user.is_authenticated:
        fota_ecu = Fota_Ecu.objects.get(ecu_id=pk)
        return render(request, 'ecus.html', {'ecus': fota_ecu})
    else:
        messages.success(
            request, "You must be logged in to view the database")
        return redirect('home')


def vehicle_database(request, pk):
    if request.user.is_authenticated:
        fota_vehicle = Fota_Vehicle.objects.get(vehicle_id=pk)
        return render(request, 'vehicles.html', {'vehicles': fota_vehicle})
    else:
        messages.success(
            request, "You must be logged in to view the database")
        return redirect('home')
