#include "segment.h"
#include <stdio.h>
#include <stdlib.h>

void split_file(const char *filename, int segment_size) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Cannot open file");
        return;
    }

    unsigned char *buffer = malloc(segment_size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(fp);
        return;
    }

    int segment_index = 0;
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, segment_size, fp)) > 0) {
        char segment_name[256];
        sprintf(segment_name, "segments/segment_%d.dat", segment_index);
        FILE *seg_fp = fopen(segment_name, "wb");
        if (!seg_fp) {
            perror("Cannot create segment file");
            break;
        }
        fwrite(buffer, 1, bytes_read, seg_fp);
        fclose(seg_fp);
        segment_index++;
    }

    free(buffer);
    fclose(fp);

    printf("File split into %d segments.\n", segment_index);
}
