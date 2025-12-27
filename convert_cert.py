# convert_der_to_c.py
with open("mosquitto.org.der", "rb") as f:
    data = f.read()

print("const uint8_t mq_client_cert[] = {")
for i, b in enumerate(data):
    end = "," if i < len(data)-1 else ""
    print(f"0x{b:02X}{end}", end="")
    if (i+1) % 16 == 0:
        print()
print("};")
print(f"const uint32_t mq_client_cert_len = {len(data)};")


# to convert from .pem to .der use the cmd
# openssl x509 -in client_key.pem -outform der -out client_key.der