#include "Animation.h"
#include "GTTime.h"
#include "GameObject.h"
#include "SkinnedMeshRenderer.h"
#include <map>
#include "Debug.h"

namespace Galaxy3D
{
    DEFINE_COM_CLASS(Animation);

    void Animation::DeepCopy(const std::shared_ptr<Object> &source)
    {
        auto src_anim = std::dynamic_pointer_cast<Animation>(source);

        Component::DeepCopy(source);

        m_states = src_anim->m_states;
        m_bones = src_anim->m_bones;

        // make sure bones are matching copy,
        // include animation and all skinned mesh renderer
        auto renderers = GetGameObject()->GetComponentsInChildren<SkinnedMeshRenderer>();

        for(auto &i : m_bones)
        {
            auto path = i.first;
            auto bone = GetTransform()->Find(path);
            auto src_bone = i.second;
            i.second = bone;

            for(auto &j : renderers)
            {
                auto &renderer_bones = j->GetBones();

                for(size_t k=0; k<renderer_bones.size(); k++)
                {
                    if(renderer_bones[k] == src_bone)
                    {
                        renderer_bones[k] = bone;
                    }
                }
            }
        }

        Stop();
    }

    void Animation::Start()
    {
    }

    void Animation::Update()
    {
        m_blends.clear();

        for(auto i = m_states.begin(); i != m_states.end(); i++)
        {
            AnimationState *state = &i->second;
            AnimationClip *c = state->clip.get();

            if(!state->enabled)
            {
                continue;
            }

            float now = GTTime::GetTime();
            float time_delta = now - state->time_last;
            state->time += time_delta * state->play_dir;
            state->time_last = now;

            if(state->fade.mode == FadeMode::In)
            {
                state->fade.weight += time_delta * (state->weight - 0) / state->fade.length;
                if(state->fade.weight >= state->weight)
                {
                    state->fade.Clear();
                }
            }
            else if(state->fade.mode == FadeMode::Out)
            {
                state->fade.weight += time_delta * (0 - state->weight) / state->fade.length;
                if(state->fade.weight <= 0)
                {
                    Stop(*state);
                }
            }

            if(	(state->play_dir == 1 && state->time > state->length) ||
                (state->play_dir == -1 && state->time < 0))
            {
                WrapMode::Enum wrap_mode = state->wrap_mode;
                if(wrap_mode == WrapMode::Default)
                {
                    wrap_mode = c->wrap_mode;
                }

                switch(wrap_mode)
                {
                    case WrapMode::Default:
                    case WrapMode::Once:
                        Stop(*state);
                        continue;

                    case WrapMode::Loop:
                        state->time = 0;
                        break;

                    case WrapMode::PingPong:
                        if(state->play_dir == 1)
                        {
                            state->play_dir = -1;
                            state->time = state->length;
                        }
                        else
                        {
                            state->play_dir = 1;
                            state->time = 0;
                        }
                        break;

                    case WrapMode::ClampForever:
                        state->time = state->length;
                        break;
                }
            }

            if(state->enabled)
            {
                Blend blend;
                blend.state = state;
                m_blends.push_back(blend);
            }
        }

        UpdateBlend();
        UpdateBones();
    }

    void Animation::UpdateBlend()
    {
        float full_weight = 1.0f;
        float remain_weight = 1.0f;
        int layer = 0x7fffffff;

        //compute weights
        m_blends.sort();
        for(auto i = m_blends.begin(); i != m_blends.end(); i++)
        {
            if(remain_weight <= 0)
            {
                i->weight = 0;
                continue;	
            }

            if(i->state->layer < layer)
            {
                full_weight = remain_weight;
            }
            layer = i->state->layer;

            float weight;
            if(i->state->fade.mode != FadeMode::None)
            {
                weight = full_weight * i->state->fade.weight;
            }
            else
            {
                weight = full_weight * i->state->weight;
            }

            {
                i++;
                if(i == m_blends.end())
                {
                    weight = remain_weight;
                }
                i--;
            }

            if(remain_weight - weight < 0)
            {
                weight = remain_weight;
            }
            remain_weight -= weight;

            i->weight = weight;
        }
    }

    void Animation::UpdateBones()
    {
        for(auto i=m_bones.begin(); i!=m_bones.end(); i++)
        {
            std::vector<Vector3> poss;
            std::vector<Quaternion> rots;
            std::vector<Vector3> scas;
            std::vector<float> weights;
            float no_effect_weight = 0;

            for(auto j=m_blends.begin(); j!=m_blends.end(); j++)
            {
                auto state = j->state;
                float weight = j->weight;

                auto find = state->clip->curves.find(i->first);
                if(find != state->clip->curves.end())
                {
                    Vector3 pos(0, 0, 0);
                    Quaternion rot(0, 0, 0, 1);
                    Vector3 sca(1, 1, 1);
                    auto &cb = find->second;

                    for(size_t k=0; k<cb.curves.size(); k++)
                    {
                        auto &curve = cb.curves[k];
                        if(!curve.keys.empty())
                        {
                            float value = curve.Evaluate(state->time);

                            auto p = (CurveProperty::Enum) k;
                            switch(p)
                            {
                                case CurveProperty::LocalPosX:
                                    pos.x = value;
                                    break;
                                case CurveProperty::LocalPosY:
                                    pos.y = value;
                                    break;
                                case CurveProperty::LocalPosZ:
                                    pos.z = value;
                                    break;

                                case CurveProperty::LocalRotX:
                                    rot.x = value;
                                    break;
                                case CurveProperty::LocalRotY:
                                    rot.y = value;
                                    break;
                                case CurveProperty::LocalRotZ:
                                    rot.z = value;
                                    break;
                                case CurveProperty::LocalRotW:
                                    rot.w = value;
                                    break;

                                case CurveProperty::LocalScaX:
                                    sca.x = value;
                                    break;
                                case CurveProperty::LocalScaY:
                                    sca.y = value;
                                    break;
                                case CurveProperty::LocalScaZ:
                                    sca.z = value;
                                    break;

								default:
									break;
                            }
                        }
                    }

                    poss.push_back(pos);
                    rots.push_back(rot);
                    scas.push_back(sca);
                    weights.push_back(weight);
                }
                else
                {
                    no_effect_weight += weight;
                }
            }

            int in_effect_count = weights.size();
            for(int j=0; j<in_effect_count; j++)
            {
                float per_add = no_effect_weight / in_effect_count;
                weights[j] += per_add;
            }

            Vector3 pos_final(0, 0, 0);
            Quaternion rot_final(0, 0, 0, 0);
            Vector3 sca_final(0, 0, 0);
            for(int j=0; j<in_effect_count; j++)
            {
                pos_final += poss[j] * weights[j];

                if(j > 0 && rots[j].Dot(rots[0]) < 0)
                {
                    rots[j] = rots[j] * -1.0f;
                }
                rot_final.x += rots[j].x * weights[j];
                rot_final.y += rots[j].y * weights[j];
                rot_final.z += rots[j].z * weights[j];
                rot_final.w += rots[j].w * weights[j];

                sca_final += scas[j] * weights[j];
            }

            if(in_effect_count > 1)
            {
                in_effect_count = in_effect_count;
            }

            if(in_effect_count > 0)
            {
                rot_final.Normalize();
                i->second->SetLocalPositionDirect(pos_final);
                i->second->SetLocalRotationDirect(rot_final);
                i->second->SetLocalScaleDirect(sca_final);
            }
        }

        GetTransform()->Changed();
    }

    AnimationState *Animation::GetAnimationState(const std::string &clip)
    {
        auto find = m_states.find(clip);
        if(find != m_states.end())
        {
            return &find->second;
        }

        return NULL;
    }

    void Animation::Stop()
    {
        for(auto &i : m_states)
        {
            Stop(i.second);
        }
    }

    void Animation::Stop(AnimationState &state)
    {
        state.enabled = false;
        state.time = 0;
        state.fade.Clear();
    }

    void Animation::Play(AnimationState &state)
    {
        state.time_last = GTTime::GetTime();
        state.enabled = true;
    }

    void Animation::CrossFade(const std::string &clip, float fade_length, PlayMode::Enum mode)
    {
        auto find = m_states.find(clip);
        if(find == m_states.end())
        {
            return;
        }

        AnimationState *state = &find->second;

        for(auto i=m_states.begin(); i!=m_states.end(); i++)
        {
            AnimationState *s = &i->second;

            if(mode == PlayMode::StopAll && state != s && s->enabled)
            {
                if(s->fade.mode == FadeMode::None)
                {
                    s->fade.mode = FadeMode::Out;
                    s->fade.length = fade_length;
                    s->fade.weight = s->weight;
                }
                else if(s->fade.mode == FadeMode::In)
                {
                    s->fade.mode = FadeMode::Out;
                    s->fade.length = fade_length;
                }
            }
            else if(mode == PlayMode::StopSameLayer && s->layer == state->layer && state != s && s->enabled)
            {
                if(s->fade.mode == FadeMode::None)
                {
                    s->fade.mode = FadeMode::Out;
                    s->fade.length = fade_length;
                    s->fade.weight = s->weight;
                }
                else if(s->fade.mode == FadeMode::In)
                {
                    s->fade.mode = FadeMode::Out;
                    s->fade.length = fade_length;
                }
            }
            else if(state == s)
            {
                if(!s->enabled)
                {
                    Play(*s);

                    s->fade.mode = FadeMode::In;
                    s->fade.length = fade_length;
                    s->fade.weight = 0;
                }
                else
                {
                    if(s->fade.mode == FadeMode::Out)
                    {
                        s->fade.mode = FadeMode::In;
                        s->fade.length = fade_length;
                    }
                }
            }
        }
    }

    bool Animation::Play(const std::string &clip, PlayMode::Enum mode)
    {
        auto find = m_states.find(clip);
        if(find == m_states.end())
        {
            return false;
        }

        AnimationState *state = &find->second;

        for(auto i=m_states.begin(); i!=m_states.end(); i++)
        {
            AnimationState *s = &i->second;

            if(mode == PlayMode::StopAll && state != s && s->enabled)
            {
                Stop(*s);
            }
            else if(mode == PlayMode::StopSameLayer && s->layer == state->layer && state != s && s->enabled)
            {
                Stop(*s);
            }
            else if(state == s && !s->enabled)
            {
                Play(*s);
            }
        }

        return true;
    }
}