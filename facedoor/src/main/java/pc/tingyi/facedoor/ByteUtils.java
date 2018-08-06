package pc.tingyi.facedoor;

public class ByteUtils {

    /**
     * hex String
     */
    public final static String HEX_DIGITS = "0123456789ABCDEF";

    /**
     * hex char array
     */
    final protected static char[] HEX_ARRAY = HEX_DIGITS.toCharArray();

    /**
     * convert bytes to hex string
     *
     * @param bytes
     * @return
     */
    public static String toHexString(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = HEX_ARRAY[v >>> 4];
            hexChars[j * 2 + 1] = HEX_ARRAY[v & 0x0F];
        }
        return new String(hexChars);
    }

    /**
     * convert bytes to hex string
     *
     * @param hexString
     * @return
     */
    public static byte[] hexToBytes(String hexString) {

        if ((hexString.length() &1) != 0) {
            hexString = "0" + hexString;
        }

        hexString = hexString.toUpperCase();

        int length = hexString.length() / 2;

        char[] hexChars = hexString.toCharArray();

        byte[] result = new byte[length];

        for (int i = 0; i < length; i++) {
            int pos = i * 2;
            result[i] = (byte) (hexChartoByte(hexChars[pos]) << 4 | hexChartoByte(hexChars[pos + 1]));

        }
        return result;
    }

    /**
     * Convert hex char to byte;
     *
     * @param c
     * @return
     */
    private static byte hexChartoByte(char c) {
        return (byte) HEX_DIGITS.indexOf(c);
    }


    /**
     * change from byte[] to int
     *
     * @param input input byte[]
     * @return int value of input
     */
    public static int toInt(byte[] input) {
        return toInt(input, 0);
    }

    /**
     * change from byte[] to int
     *
     * @param input input byte[]
     * @param offset begin offset of the input byte[]
     * @return int value of input
     */
    public static int toInt(byte[] input, int offset) {
        int result = (input[offset + 3] & 0xff) << 24;
        result += (input[offset + 2] & 0xff) << 16;
        result += (input[offset + 1] & 0xff) << 8;
        result += (input[offset + 0] & 0xff);
        return result;
    }

    /**
     * change from byte[] to int
     *
     * @param input input byte[]
     * @return int value of input
     */
    public static int toUInt16(byte[] input) {
        return toUInt16(input, 0);
    }

    /**
     * change from byte[] to int
     *
     * @param input input byte[]
     * @param offset begin offset of the input byte[]
     * @return int value of input
     */
    public static int toUInt16(byte[] input, int offset) {
        int result = (input[offset + 1] & 0xff) << 8;
        result += (input[offset + 0] & 0xff);
        return result;
    }

    /**
     * write a int to dest byte[]
     *
     * @param input int to be written
     * @param dest  dest byte[] to be written
     * @param offset dest byte[] begin offset to be written
     */
    public static void writeUInt16(int input, byte[] dest, int offset) {
        dest[offset + 0] = (byte) (input & 0xff);
        dest[offset + 1] = (byte) ((input >> 8) & 0xff);
    }

    /**
     * write a int to dest byte[]
     *
     * @param input int to be written
     * @param dest  dest byte[] to be written
     * @param offset dest byte[] begin offset to be written
     */
    public static void write(int input, byte[] dest, int offset) {
        dest[offset + 0] = (byte) (input & 0xff);
        dest[offset + 1] = (byte) ((input >> 8) & 0xff);
        dest[offset + 2] = (byte) ((input >> 16) & 0xff);
        dest[offset + 3] = (byte) ((input >> 24) & 0xff);
    }

    /**
     * write a byte[] to dest byte[]
     *
     * @param input int to be written
     * @param dest  dest byte[] to be written
     * @param offset dest byte[] begin offset to be written
     */
    public static void write(byte[] input, int length, byte[] dest, int offset) {
        for (int i = 0; i < length; i++) {
            dest[offset + i] = input[i];
        }
    }

    /**
     * copy from input byte[] into dest byte[]
     *
     * @param input
     * @param inputOffset
     * @param length
     * @param dest
     * @param destOffset
     */
    public static void copy(byte[] input, int inputOffset, int length, byte[] dest, int destOffset) {
        for (int i = 0; i < length; i++) {
            dest[destOffset + i] = input[inputOffset + i];
        }
    }

    /**
     * convert int into byte[]
     *
     * @param idx
     * @return
     */
    public static byte[] toBytes(int idx) {
        byte[] result = new byte[4];
        write(idx, result, 0);
        return result;
    }
}
