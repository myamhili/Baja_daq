float Calculate_RPM(uint32_t delta_ticks, float ppr) {
    if (delta_ticks == 0) return 0.0f; 
    return (60.0f * TIMER_FREQ_HZ) / ((float)delta_ticks * ppr);
}

float Calculate_Ground_Speed_KMH(float secondary_rpm) {
    float wheel_rpm = secondary_rpm / GEARBOX_RATIO;
    return (wheel_rpm * 3.14159f * TIRE_DIAMETER_M * 60.0f) / 1000.0f;
}