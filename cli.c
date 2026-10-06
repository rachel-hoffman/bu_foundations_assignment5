#include "kernel.h"
#include <string.h>
 
int generate_pagefault() {
    long pagesize = sysconf(_SC_PAGESIZE);

    char *p = mmap(
        NULL,
        pagesize * 2,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (p == MAP_FAILED) {
        return -1;
    }

    // Touch two previously-unaccessed pages
    p[0] = 'A';
    p[pagesize] = 'B';

    munmap(p, pagesize * 2);

    return 1;
}

int main(int argc, char** argv){

    if(argc != 6) {
        printf("Incorrect number of arguments. Expected: ./cli <MODE=kernel|mmap|convert|uconvert|fault> <input_image> <width> <height> <output_image_path>\n");
        return -1;
    }

    // Parse input
    char* MODE = argv[1];
    int width = atoi(argv[3]);
    int height = atoi(argv[4]);

    // Input img
    struct image* img = malloc(sizeof(struct image));
    if (img == NULL) {
        printf("Failed to allocate memory for image\n");
        return -1;
    }
    img->width  = width;
    img->height = height;
    img->pixels = NULL;


    // Output img
    struct image* img_out = NULL;
    int mmapped = 0;
    
    int kernel[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
    

    // TODO: call correct function based on mode
    if (strcmp(MODE, "kernel") == 0) {
        if(loadimage(argv[2], img) != 0) { // Catch error code
            printf("Failed to load image %s\n", argv[2]);
            free(img);
            return -1;
        }
        img_out = apply_kernel(img, *kernel, 3, 1 / 9.0f);
        saveimage(argv[5], img_out);

    } else if (strcmp(MODE, "mmap") == 0) {
        if(loadimage_mmap(argv[2], img) != 0) {
            printf("Failed to load image %s\n", argv[2]);
            free(img);
            return -1;
        }
        mmapped = 1;
        img_out = apply_kernel(img, *kernel, 3, 1 / 9.0f);
        saveimage_mmap(argv[5], img_out);

    } else if (strcmp(MODE, "convert") == 0) {
        if(loadimage(argv[2], img) != 0) { 
            printf("Failed to load image %s\n", argv[2]);
            free(img);
            return -1;
        }
        saveimage_mmap(argv[5], img);

    } else if (strcmp(MODE, "uconvert") == 0) {
        if(loadimage_mmap(argv[2], img) != 0) {
            printf("Failed to load image %s\n", argv[2]);
            free(img);
            return -1;
        }
        mmapped = 1;
        saveimage(argv[5], img);

    } else if (strcmp(MODE, "fault") == 0) {
        return generate_pagefault();
    } else {
        printf("Invalid mode. Expected: kernel, mmap, convert, uconvert, fault\n");
        free(img);
        return -1;
    }
    // TODO: allocate the space needed for one image and load the image

    // Cleanup
    
    if (img->pixels != NULL) {
        if (mmapped){
            unmap_image(img);
        } else {
            free(img->pixels);
        }
    }
    free(img);
    if (img_out != NULL) {
        free(img_out->pixels);
        free(img_out);
    }
    

    return 0;
    
}
