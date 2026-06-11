.text
memcpy: // x0 is source, x1 is destination, x2 is number of words
        cmp     x2, #0
        beq     memcpy_end      // exit if done copying
        ldr     x9, [x0]        // load into register 9
        str     x9, [x1]        // store into destination
        add     x0, x0, #8      // move to next word
        add     x1, x1, #8
        sub     x2, x2, #1
        b       memcpy
memcpy_end:
        ret
