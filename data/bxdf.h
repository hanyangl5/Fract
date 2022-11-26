
Fresnel(float cos_i, float cos_t, float eta_i, float eta_t) {
    float rs = (eta_i * cos_i - eta_t * cos_t) / (eta_i * cos_i + eta_t * cos_t);
    float rp = (eta_i * cos_t - eta_t * cos_i) / (eta_i * cos_t + eta_t * cos_i);
    float ts = (2 * eta_i * cos_i) / (eta_i * cos_i + eta_t * cos_t);
    float tp = (2 * eta_t * cos_t) / (eta_i * cos_t + eta_t * cos_i);
    rs = rs * rs;
    rp = rp * rp;
    ts = ts * ts;
    tp = tp * tp;
    float reflection = (rs + rp) / 2;
    float transimission = (ts + tp) / 2;
}