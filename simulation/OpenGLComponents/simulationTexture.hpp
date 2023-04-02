#include <glad/gl.h>

namespace openGLComponents{

    /*
        This isnt called just "texture" because it is not designed to be used for anything other than the simulation,
        and I dont want it to be confusing :)

        Basically this class is useless outside of exactly what im making it for.
    */
    class simulationTexture{
        private:
            unsigned int* textures;

            void makeTextures(unsigned int* textures, unsigned int n, unsigned int res){
                glGenTextures(n, textures);
                for (int i = 0; i < n; i++) {
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, textures[i]);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, res, res, 0, GL_RGBA, GL_FLOAT, NULL);
                }
            }

        public:
            void init(unsigned int res){
                this->textures = new unsigned int[0];
                this->makeTextures(this->textures, 0, res);
            }

            void bind(unsigned int idx, unsigned int texid, unsigned int unit){
                glActiveTexture(GL_TEXTURE0 + texid);
                glBindTextureUnit(unit, this->textures[idx]);
                glBindImageTexture(unit, this->textures[idx], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
            }

    };

}