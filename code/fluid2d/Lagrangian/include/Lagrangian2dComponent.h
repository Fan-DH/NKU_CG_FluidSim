#pragma once
#ifndef __LAGRANGIAN_2D_COMPONENT_H__
#define __LAGRANGIAN_2D_COMPONENT_H__

#include "Renderer.h"
#include "Solver.h"
#include "ParticleSystem2d.h"

#include "Component.h"
#include "Configure.h"
#include "Logger.h"

namespace FluidSimulation
{
    namespace Lagrangian2d
    {
        // �������շ�����ģ�������
        // ��������ϵͳ�����������Ⱦ��
        class Lagrangian2dComponent : public Glb::Component
        {
        public:
            Renderer *renderer;        // ��Ⱦ��
            Solver *solver;            // �����
            ParticleSystem2d *ps;      // ����ϵͳ

            Lagrangian2dComponent(char *description, int id)
            {
                this->description = description;
                this->id = id;
                renderer = NULL;
                solver = NULL;
                ps = NULL;
            }

            virtual void shutDown();           // �ر����,�ͷ���Դ
            virtual void init();               // ��ʼ�����
            virtual void simulate();           // ִ��һ��ģ��
            virtual GLuint getRenderedTexture();  // ��ȡ��Ⱦ���
        };
    }
}

#endif