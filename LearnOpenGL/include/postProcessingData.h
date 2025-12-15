//
// Created by Aentro on 2025/12/15.
//

#ifndef LEARNOPENGL_CLION_POSTPROCESSINGDATA_H
#define LEARNOPENGL_CLION_POSTPROCESSINGDATA_H
struct PostProcessingData {
    float exposure = 1.0f;
    int amount = 10;
    float gamma = 1.2f;
    float bloomStrength = 0.4;
    PostProcessingData() = default;
};
#endif //LEARNOPENGL_CLION_POSTPROCESSINGDATA_H