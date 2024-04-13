import random

# Function to generate a random KV request trace
def generate_kv_request_trace(num_requests, output_file):
    put_keys = set()  # Set to store the keys that have been PUT before
    with open(output_file, 'w') as file:
        for i in range(num_requests):
            # Generate a random choice between PUT and GET
            request_type = random.choice(['PUT', 'GET'])

            if request_type == 'PUT':
                # Generate a random 32-bit integer key
                key = random.randint(0, 2**15 - 1)
                # Generate a random string value less than 4096 bytes
                value_length = random.randint(1, 4095)  # Random length less than 4096
                value = ''.join(random.choices('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789', k=value_length))
                # Write the PUT request to the file
                file.write(f"{request_type} {key} {value}\n")
                # Add the key to the set of put_keys
                put_keys.add(key)
            elif request_type == 'GET':
                if put_keys:  # Check if there are keys available for GET
                    # Select a random key from the set of put_keys
                    key = random.choice(list(put_keys))
                    # Write the GET request to the file
                    file.write(f"{request_type} {key}\n")
                else:
                    # If no keys are available for GET, generate a PUT request instead
                    key = random.randint(0, 2**15 - 1)
                    value_length = random.randint(1, 4095)
                    value = ''.join(random.choices('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789', k=value_length))
                    file.write(f"PUT {key} {value}\n")
                    put_keys.add(key)

# Example usage
num_requests = 100
output_file = "./kv_trace2.txt"
generate_kv_request_trace(num_requests, output_file)
print(f"KV request trace generated and saved to {output_file}")
