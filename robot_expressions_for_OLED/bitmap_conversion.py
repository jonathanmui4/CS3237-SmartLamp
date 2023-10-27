from PIL import Image
from glob import glob
from os.path import basename
def image_to_binary_array(image_path):
    try:
        # Open the image
        img = Image.open(image_path)

        # Convert the image to grayscale
        img = img.convert('L')

        # Initialize an empty list to store the binary values
        binary_data = []

        # Iterate over the pixels in the image
        for pixel in img.getdata():
            # Check if all RGB values are 0
            if pixel == 0:
                binary_data.append(0)
            else:
                binary_data.append(1)
        print(len(binary_data))
        # Convert the binary data to a hex string
        hex_data = ""
        for i in range(0, len(binary_data), 8):
            byte = binary_data[i:i+8]
            hex_byte = ''.join([str(bit) for bit in byte])
            hex_data += '0x'+hex(int(hex_byte, 2))[2:].zfill(2)+', '

        return hex_data

    except Exception as e:
        print(f"An error occurred: {str(e)}")

if __name__ == "__main__":
    folder_name = "/Users/haihongyu/Downloads/expressions"
    file = open("/Users/haihongyu/Downloads/expressions/expressions.c", 'w')
    file.write("#include<avr/pgmspace.h>\n")
    for file_idx, file_name in enumerate(glob('%s/*.png' % folder_name)):
        frame_name = basename(file_name)[:-4]
        print(file_name, file_idx, frame_name)
        hex_data = image_to_binary_array(file_name)
        output_path = folder_name + '/' + frame_name + '.txt'
        file.write("const unsigned char %s []PROGMEM = {" % frame_name)
        file.write(hex_data[:-2])
        file.write("};\n")