#pragma once
#ifndef __LAGRANGIAN_2D_RENDERER_H__
#define __LAGRANGIAN_2D_RENDERER_H__

#include <glad/glad.h>
#include <glfw3.h>

#include <chrono>
#include <vector>
#include "Shader.h"
#include <glm/glm.hpp>

#include "ParticleSystem2d.h"
#include "Configure.h"
#include <Logger.h>

namespace FluidSimulation
{
    namespace Lagrangian2d
    {
        // �������շ�������Ⱦ����
        // ��������ϵͳ���ӻ�
        class Renderer
        {
        public:
            Renderer();

            int32_t init();                                // ��ʼ����Ⱦ��
            void draw(ParticleSystem2d& ps);               // ��������ϵͳ
            GLuint getRenderedTexture();                   // ��ȡ��Ⱦ���������ID

        private:
            Glb::Shader *shader = nullptr;                 // ��ɫ������

            GLuint VAO = 0;                                // �����������
            GLuint positionVBO = 0;                        // λ�ö��㻺�����
            GLuint densityVBO = 0;                         // �ܶȶ��㻺�����
            GLuint markVBO = 0;

            GLuint FBO = 0;                                // ֡�������
            GLuint textureID = 0;                          // ��Ⱦ����ID
            GLuint RBO = 0;                                // ��Ⱦ�������

            size_t particleNum = 0;                        // ��������
        };
    }
}

#endif
