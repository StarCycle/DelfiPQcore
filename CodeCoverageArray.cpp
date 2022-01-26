unsigned char coverageArray[253] = {0};

void CodeCount(int label)
{
    coverageArray[label/8] |= 1<<(label%8);
}
