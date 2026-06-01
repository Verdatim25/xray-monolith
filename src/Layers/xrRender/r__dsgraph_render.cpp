#include "stdafx.h"

#include "../../xrEngine/render.h"
#include "../../xrEngine/irenderable.h"
#include "../../xrEngine/igame_persistent.h"
#include "../../xrEngine/environment.h"
#include "../../xrEngine/CustomHUD.h"

#include "FBasicVisual.h"
#include "CHudInitializer.h"

#include "fhierrarhyvisual.h"
#include "SkeletonCustom.h"
#include "../../xrEngine/fmesh.h"
#include "flod.h"

#include "../../xrEngine/xr_object.h"

using namespace R_dsgraph;

extern float r_ssaDISCARD;
extern float r_ssaDONTSORT;
extern float r_ssaHZBvsTEX;
extern float r_ssaGLOD_start, r_ssaGLOD_end;

ICF float calcLOD(float ssa/*fDistSq*/, float R)
{
	return _sqrt(clampr((ssa - r_ssaGLOD_end) / (r_ssaGLOD_start - r_ssaGLOD_end), 0.f, 1.f));
}

template<typename T, bool Reverse>
void CDSGraphManager::r_dsgraph_render_graph_sorted(R_dsgraph::mapDSGraphItems<T, Reverse>& graph, bool _clear)
{
    if (graph.empty())
        return;

    std::sort(graph.begin(), graph.end());

	for (auto& item : graph)
	{
		dxRender_Visual* V = item.pVisual;
		VERIFY(V && V->shader._get());
		RCache.set_Element(item.pSE);
		RCache.set_xform_world(*item.pMatrix);
		RImplementation.apply_object(item.pObject);
		RImplementation.apply_lmaterial();
<<<<<<< HEAD
		//if (item.b_hud_mode)
		//{
		//	//new feature
		//}
		V->Render(calcLOD(item.ssa, V->vis.sphere.R));
=======

		float LOD = calcLOD(Ni.ssa, Ni.pVisual->vis.sphere.R);
#ifdef USE_DX11
		RCache.LOD.set_LOD(LOD);
#endif
		Ni.pVisual->Render(LOD);
	}
	N.clear();
}

// ALPHA
void __fastcall sorted_L1(mapSorted_Node* N)
{
	VERIFY(N);
	dxRender_Visual* V = N->val.pVisual;
	VERIFY(V && V->shader._get());
	RCache.set_Element(N->val.se);
	RCache.set_xform_world(N->val.Matrix);
	RImplementation.apply_object(N->val.pObject);
	RImplementation.apply_lmaterial();
	V->Render(calcLOD(N->key, V->vis.sphere.R));
}

void __fastcall water_node_ssr(mapSorted_Node* N)
{
#ifdef USE_DX11
	VERIFY(N);
	dxRender_Visual* V = N->val.pVisual;
	VERIFY(V);

	RCache.set_Shader(RImplementation.Target->s_ssfx_water_ssr);

	RCache.set_xform_world(N->val.Matrix);
	RImplementation.apply_object(N->val.pObject);
	RImplementation.apply_lmaterial();

	RCache.set_c("cam_pos", RImplementation.Target->GetPrevious()->Position_previous.x, RImplementation.Target->GetPrevious()->Position_previous.y, RImplementation.Target->GetPrevious()->Position_previous.z, 0.0f);

	// Previous matrix data
	RCache.set_c("m_current", RImplementation.Target->GetPrevious()->Matrix_current);
	RCache.set_c("m_previous", RImplementation.Target->GetPrevious()->Matrix_previous);

	V->Render(calcLOD(N->key, V->vis.sphere.R));
#endif
}

void __fastcall water_node(mapSorted_Node* N)
{
	VERIFY(N);
	dxRender_Visual* V = N->val.pVisual;
	VERIFY(V);

#ifdef USE_DX11
	if (RImplementation.o.ssfx_water)
	{
		RCache.set_Shader(RImplementation.Target->s_ssfx_water);
	}
#endif

	RCache.set_xform_world(N->val.Matrix);
	RImplementation.apply_object(N->val.pObject);
	RImplementation.apply_lmaterial();

	// Wind settings
	float WindDir = g_pGamePersistent->Environment().CurrentEnv->wind_direction;
	float WindVel = g_pGamePersistent->Environment().CurrentEnv->wind_velocity;
	RCache.set_c("wind_setup", WindDir, WindVel, 0, 0);

	V->Render(calcLOD(N->key, V->vis.sphere.R));
}

/*void __fastcall hud_node(mapSorted_Node* N)
{
	VERIFY(N);
	dxRender_Visual* V = N->val.pVisual;
	VERIFY(V && V->shader._get());
	RCache.set_xform_world(N->val.Matrix);

#ifdef USE_DX11

	if (N->val.se->passes[0]->ps->hud_disabled)
		return;

	int skinning = N->val.se->passes[0]->vs->skinning;
	RCache.set_Shader(RImplementation.Target->s_ssfx_hud[skinning]);

	RImplementation.Target->GetPrevious()->Matrix_HUD_previous.set(N->val.PrevMatrix);
	N->val.PrevMatrix.set(RCache.xforms.m_wvp);

	RImplementation.Target->GetPrevious()->RVelocity = true;

#endif

	V->Render(calcLOD(N->key, V->vis.sphere.R));

#ifdef USE_DX11
	RImplementation.Target->GetPrevious()->RVelocity = false;
#endif
}*/

IC bool cmp_vs_nrm(mapNormalVS::TNode* N1, mapNormalVS::TNode* N2)
{
	return (N1->val.ssa > N2->val.ssa);
}

IC bool cmp_vs_mat(mapMatrixVS::TNode* N1, mapMatrixVS::TNode* N2)
{
	return (N1->val.ssa > N2->val.ssa);
}

IC bool cmp_ps_nrm(mapNormalPS::TNode* N1, mapNormalPS::TNode* N2)
{
#ifdef USE_DX11
	return (N1->val.mapCS.ssa > N2->val.mapCS.ssa);
#else
	return (N1->val.ssa > N2->val.ssa);
#endif
}

IC bool cmp_ps_mat(mapMatrixPS::TNode* N1, mapMatrixPS::TNode* N2)
{
#ifdef USE_DX11
	return (N1->val.mapCS.ssa > N2->val.mapCS.ssa);
#else
	return (N1->val.ssa > N2->val.ssa);
#endif
}

#if defined(USE_DX10) || defined(USE_DX11)
IC bool cmp_gs_nrm(mapNormalGS::TNode* N1, mapNormalGS::TNode* N2) { return (N1->val.ssa > N2->val.ssa); }
IC bool cmp_gs_mat(mapMatrixGS::TNode* N1, mapMatrixGS::TNode* N2) { return (N1->val.ssa > N2->val.ssa); }
#endif	//	USE_DX10

IC bool cmp_cs_nrm(mapNormalCS::TNode* N1, mapNormalCS::TNode* N2) { return (N1->val.ssa > N2->val.ssa); }
IC bool cmp_cs_mat(mapMatrixCS::TNode* N1, mapMatrixCS::TNode* N2) { return (N1->val.ssa > N2->val.ssa); }

IC bool cmp_states_nrm(mapNormalStates::TNode* N1, mapNormalStates::TNode* N2) { return (N1->val.ssa > N2->val.ssa); }
IC bool cmp_states_mat(mapMatrixStates::TNode* N1, mapMatrixStates::TNode* N2) { return (N1->val.ssa > N2->val.ssa); }

IC bool cmp_textures_lex2_nrm(mapNormalTextures::TNode* N1, mapNormalTextures::TNode* N2)
{
	STextureList* t1 = N1->key;
	STextureList* t2 = N2->key;
	if ((*t1)[0] < (*t2)[0]) return true;
	if ((*t1)[0] > (*t2)[0]) return false;
	if ((*t1)[1] < (*t2)[1]) return true;
	else return false;
}

IC bool cmp_textures_lex2_mat(mapMatrixTextures::TNode* N1, mapMatrixTextures::TNode* N2)
{
	STextureList* t1 = N1->key;
	STextureList* t2 = N2->key;
	if ((*t1)[0] < (*t2)[0]) return true;
	if ((*t1)[0] > (*t2)[0]) return false;
	if ((*t1)[1] < (*t2)[1]) return true;
	else return false;
}

IC bool cmp_textures_lex3_nrm(mapNormalTextures::TNode* N1, mapNormalTextures::TNode* N2)
{
	STextureList* t1 = N1->key;
	STextureList* t2 = N2->key;
	if ((*t1)[0] < (*t2)[0]) return true;
	if ((*t1)[0] > (*t2)[0]) return false;
	if ((*t1)[1] < (*t2)[1]) return true;
	if ((*t1)[1] > (*t2)[1]) return false;
	if ((*t1)[2] < (*t2)[2]) return true;
	else return false;
}

IC bool cmp_textures_lex3_mat(mapMatrixTextures::TNode* N1, mapMatrixTextures::TNode* N2)
{
	STextureList* t1 = N1->key;
	STextureList* t2 = N2->key;
	if ((*t1)[0] < (*t2)[0]) return true;
	if ((*t1)[0] > (*t2)[0]) return false;
	if ((*t1)[1] < (*t2)[1]) return true;
	if ((*t1)[1] > (*t2)[1]) return false;
	if ((*t1)[2] < (*t2)[2]) return true;
	else return false;
}

IC bool cmp_textures_lexN_nrm(mapNormalTextures::TNode* N1, mapNormalTextures::TNode* N2)
{
	STextureList* t1 = N1->key;
	STextureList* t2 = N2->key;
	return std::lexicographical_compare(t1->begin(), t1->end(), t2->begin(), t2->end());
}

IC bool cmp_textures_lexN_mat(mapMatrixTextures::TNode* N1, mapMatrixTextures::TNode* N2)
{
	STextureList* t1 = N1->key;
	STextureList* t2 = N2->key;
	return std::lexicographical_compare(t1->begin(), t1->end(), t2->begin(), t2->end());
}

IC bool cmp_textures_ssa_nrm(mapNormalTextures::TNode* N1, mapNormalTextures::TNode* N2)
{
	return (N1->val.ssa > N2->val.ssa);
}

IC bool cmp_textures_ssa_mat(mapMatrixTextures::TNode* N1, mapMatrixTextures::TNode* N2)
{
	return (N1->val.ssa > N2->val.ssa);
}

void sort_tlist_nrm
(
	xr_vector<mapNormalTextures::TNode*,render_alloc<mapNormalTextures::TNode*>>& lst,
	xr_vector<mapNormalTextures::TNode*,render_alloc<mapNormalTextures::TNode*>>& temp,
	mapNormalTextures& textures,
	BOOL bSSA
)
{
	int amount = textures.begin()->key->size();
	if (bSSA)
	{
		if (amount <= 1)
		{
			// Just sort by SSA
			textures.getANY_P(lst);
			std::sort(lst.begin(), lst.end(), cmp_textures_ssa_nrm);
		}
		else
		{
			// Split into 2 parts
			mapNormalTextures::TNode* _it = textures.begin();
			mapNormalTextures::TNode* _end = textures.end();
			for (; _it != _end; _it++)
			{
				if (_it->val.ssa > r_ssaHZBvsTEX) lst.push_back(_it);
				else temp.push_back(_it);
			}

			// 1st - part - SSA, 2nd - lexicographically
			std::sort(lst.begin(), lst.end(), cmp_textures_ssa_nrm);
			if (2 == amount) std::sort(temp.begin(), temp.end(), cmp_textures_lex2_nrm);
			else if (3 == amount) std::sort(temp.begin(), temp.end(), cmp_textures_lex3_nrm);
			else std::sort(temp.begin(), temp.end(), cmp_textures_lexN_nrm);

			// merge lists
			lst.insert(lst.end(), temp.begin(), temp.end());
		}
	}
	else
	{
		textures.getANY_P(lst);
		if (2 == amount) std::sort(lst.begin(), lst.end(), cmp_textures_lex2_nrm);
		else if (3 == amount) std::sort(lst.begin(), lst.end(), cmp_textures_lex3_nrm);
		else std::sort(lst.begin(), lst.end(), cmp_textures_lexN_nrm);
	}
}

void sort_tlist_mat
(
	xr_vector<mapMatrixTextures::TNode*,render_alloc<mapMatrixTextures::TNode*>>& lst,
	xr_vector<mapMatrixTextures::TNode*,render_alloc<mapMatrixTextures::TNode*>>& temp,
	mapMatrixTextures& textures,
	BOOL bSSA
)
{
	int amount = textures.begin()->key->size();
	if (bSSA)
	{
		if (amount <= 1)
		{
			// Just sort by SSA
			textures.getANY_P(lst);
			std::sort(lst.begin(), lst.end(), cmp_textures_ssa_mat);
		}
		else
		{
			// Split into 2 parts
			mapMatrixTextures::TNode* _it = textures.begin();
			mapMatrixTextures::TNode* _end = textures.end();
			for (; _it != _end; _it++)
			{
				if (_it->val.ssa > r_ssaHZBvsTEX) lst.push_back(_it);
				else temp.push_back(_it);
			}

			// 1st - part - SSA, 2nd - lexicographically
			std::sort(lst.begin(), lst.end(), cmp_textures_ssa_mat);
			if (2 == amount) std::sort(temp.begin(), temp.end(), cmp_textures_lex2_mat);
			else if (3 == amount) std::sort(temp.begin(), temp.end(), cmp_textures_lex3_mat);
			else std::sort(temp.begin(), temp.end(), cmp_textures_lexN_mat);

			// merge lists
			lst.insert(lst.end(), temp.begin(), temp.end());
		}
	}
	else
	{
		textures.getANY_P(lst);
		if (2 == amount) std::sort(lst.begin(), lst.end(), cmp_textures_lex2_mat);
		else if (3 == amount) std::sort(lst.begin(), lst.end(), cmp_textures_lex3_mat);
		else std::sort(lst.begin(), lst.end(), cmp_textures_lexN_mat);
	}
}

void R_dsgraph_structure::r_dsgraph_render_graph(u32 _priority, bool _clear)
{
	//PIX_EVENT(r_dsgraph_render_graph);
	Device.Statistic->RenderDUMP.Begin();

	// **************************************************** NORMAL
	// Perform sorting based on ScreenSpaceArea
	// Sorting by SSA and changes minimizations
	{
		RCache.set_xform_world(Fidentity);

		// Render several passes
		for (u32 iPass = 0; iPass < SHADER_PASSES_MAX; ++iPass)
		{
			//mapNormalVS&	vs				= mapNormal	[_priority];
			mapNormalVS& vs = mapNormalPasses[_priority][iPass];
			vs.getANY_P(nrmVS);
			std::sort(nrmVS.begin(), nrmVS.end(), cmp_vs_nrm);
			for (u32 vs_id = 0; vs_id < nrmVS.size(); vs_id++)
			{
				mapNormalVS::TNode* Nvs = nrmVS[vs_id];
				RCache.set_VS(Nvs->key);

#if defined(USE_DX10) || defined(USE_DX11)
				//	GS setup
				mapNormalGS& gs = Nvs->val;
				gs.ssa = 0;

				gs.getANY_P(nrmGS);
				std::sort(nrmGS.begin(), nrmGS.end(), cmp_gs_nrm);
				for (u32 gs_id = 0; gs_id < nrmGS.size(); gs_id++)
				{
					mapNormalGS::TNode* Ngs = nrmGS[gs_id];
					RCache.set_GS(Ngs->key);

					mapNormalPS& ps = Ngs->val;
					ps.ssa = 0;
#else	//	USE_DX10
				mapNormalPS& ps = Nvs->val;
				ps.ssa = 0;
#endif	//	USE_DX10

				ps.getANY_P(nrmPS);
				std::sort(nrmPS.begin(), nrmPS.end(), cmp_ps_nrm);
				for (u32 ps_id = 0; ps_id < nrmPS.size(); ps_id++)
				{
					mapNormalPS::TNode* Nps = nrmPS[ps_id];
					RCache.set_PS(Nps->key);
#ifdef USE_DX11
						mapNormalCS& cs = Nps->val.mapCS;
						cs.ssa = 0;
						RCache.set_HS(Nps->val.hs);
						RCache.set_DS(Nps->val.ds);
#else
					mapNormalCS& cs = Nps->val;
					cs.ssa = 0;
#endif
					cs.getANY_P(nrmCS);
					std::sort(nrmCS.begin(), nrmCS.end(), cmp_cs_nrm);
					for (u32 cs_id = 0; cs_id < nrmCS.size(); cs_id++)
					{
						mapNormalCS::TNode* Ncs = nrmCS[cs_id];
						RCache.set_Constants(Ncs->key);

						mapNormalStates& states = Ncs->val;
						states.ssa = 0;
						states.getANY_P(nrmStates);
						std::sort(nrmStates.begin(), nrmStates.end(), cmp_states_nrm);
						for (u32 state_id = 0; state_id < nrmStates.size(); state_id++)
						{
							mapNormalStates::TNode* Nstate = nrmStates[state_id];
							RCache.set_States(Nstate->key);

							mapNormalTextures& tex = Nstate->val;
							tex.ssa = 0;
							sort_tlist_nrm(nrmTextures, nrmTexturesTemp, tex, true);
							for (u32 tex_id = 0; tex_id < nrmTextures.size(); tex_id++)
							{
								mapNormalTextures::TNode* Ntex = nrmTextures[tex_id];
								RCache.set_Textures(Ntex->key);
								RImplementation.apply_lmaterial();

								mapNormalItems& items = Ntex->val;
								items.ssa = 0;
								mapNormal_Render(items);
								if (_clear) items.clear();
							}
							nrmTextures.clear();
							nrmTexturesTemp.clear();
							if (_clear) tex.clear();
						}
						nrmStates.clear();
						if (_clear) states.clear();
					}
					nrmCS.clear();
					if (_clear) cs.clear();
				}
				nrmPS.clear();
				if (_clear) ps.clear();
#if defined(USE_DX10) || defined(USE_DX11)
				}
				nrmGS.clear();
				if (_clear) gs.clear();
#endif	//	USE_DX10
			}
			nrmVS.clear();
			if (_clear) vs.clear();
		}
>>>>>>> april26
	}

	if (_clear)
		graph.clear();

	RCache.set_xform_world(Fidentity);
}

void CDSGraphManager::r_dsgraph_render_graph(RenderQueueArray& queues, u32 _priority, bool _clear, bool static_geometry)
{
	RCache.set_xform_world(Fidentity);

	for (u32 iPass = 0; iPass < SHADER_PASSES_MAX; ++iPass)
	{
		auto& queue = queues[_priority][iPass];
		if (queue.empty())
			continue;

		// 1. Sort by generated sort key to replicate previous fixed map behaviour
		if (queue.size() < 4096)
            std::sort(queue.begin(), queue.end());
		else
            xr_parallel_sort(queue.begin(), queue.end());

		// 2. Render
		vs_type pVS = nullptr;

#if defined(USE_DX10) || defined(USE_DX11)
		gs_type pGS = nullptr;
#endif

		ps_type pPS = nullptr;

#ifdef USE_DX11
		hs_type pHS = nullptr;
		ds_type pDS = nullptr;
#endif
		
		R_constant_table* pCS = nullptr;
		ID3DState* pState = nullptr;
		STextureList* pTextures = nullptr;

        u64 high = 0;

        for (auto& packet : queue)
        {
            auto& currentKey = packet.sortKey;
            if (currentKey.high != high)
            {
                high = currentKey.high;

                if (packet.pState != pState)
                {
                    pState = packet.pState;
                    RCache.set_States(pState);
                }

#if defined(USE_DX10) || defined(USE_DX11)
                if (packet.pGS != pGS)
                {
                    pGS = packet.pGS;
                    RCache.set_GS(pGS);
                }
#endif

#ifdef USE_DX11
                if (packet.pHS != pHS)
                {
                    pHS = packet.pHS;
                    RCache.set_HS(pHS);
                }
                if (packet.pDS != pDS)
                {
                    pDS = packet.pDS;
                    RCache.set_DS(pDS);
                }
#endif
            }

            // Compare low key stuff regardless, too high collision probability
            if (packet.pVS != pVS)
            {
                pVS = packet.pVS;
                RCache.set_VS(pVS);
            }

            if (packet.pPS != pPS)
            {
                pPS = packet.pPS;
                RCache.set_PS(pPS);
            }

            if (packet.pCS != pCS)
            {
                pCS = packet.pCS;
                RCache.set_Constants(pCS);
            }

            if (packet.pTextures != pTextures)
            {
                pTextures = packet.pTextures;
                RCache.set_Textures(pTextures);
                RImplementation.apply_lmaterial();
            }

			auto& item = packet.item;
			if (!static_geometry)
			{
				RCache.set_xform_world(*item.pMatrix);
				RImplementation.apply_object(item.pObject);
				RImplementation.apply_lmaterial();
			}

			float LOD = calcLOD(item.ssa, item.pVisual->vis.sphere.R);
#ifdef USE_DX11
			RCache.LOD.set_LOD(LOD);
#endif
			item.pVisual->Render(LOD);
		}

		if (_clear)
			queue.clear();
	}
}

//////////////////////////////////////////////////////////////////////////
// HUD render
void CDSGraphManager::r_dsgraph_render_hud()
{
	PROF_EVENT("r_dsgraph_render_hud");
	CHudInitializer initializer(true);

	// Rendering
	RImplementation.rmNear();
	r_dsgraph_render_graph_sorted(RGraph.mapHUD);

	RImplementation.rmNormal();

<<<<<<< HEAD
#if defined(USE_DX11) //  Redotix99: for 3D Shader Based Scopes 		
	if (scope_3D_fake_enabled)
	{
		RCache.set_RT(RImplementation.Target->rt_ssfx_temp->pRT, 3); // Render scope_3D to any buffer

		r_dsgraph_render_graph_sorted(RGraph.mapScopeHUD);

		if (!RImplementation.o.ssfx_motionvectors)
			RCache.set_RT(NULL, 3);
		else
			RCache.set_RT(RImplementation.Target->rt_ssfx_motion_vectors->pRT, 3);
	}
#endif
=======
		rmNormal();
>>>>>>> april26

	if (RGraph.mapCamAttached.size())
	{
		RImplementation.rmNear();

		// Change projection
		initializer.SetCamMode();

		// Rendering
		r_dsgraph_render_graph_sorted(RGraph.mapCamAttached);

		RImplementation.rmNormal();
	}
}

void CDSGraphManager::r_dsgraph_render_hud_ui()
{
	PROF_EVENT("r_dsgraph_render_hud_ui");
	CHudInitializer initializer(true);

	RImplementation.rmNear();
	g_hud->RenderActiveItemUI();
	RImplementation.rmNormal();
}

void CDSGraphManager::r_dsgraph_render_cam_ui()
{
	PROF_EVENT("r_dsgraph_render_cam_ui");

	// Change projection
	CHudInitializer initializer(2);
	
	// Rendering
	RImplementation.rmNear();
	g_hud->RenderCamAttachedUI();
	RImplementation.rmNormal();
}

//////////////////////////////////////////////////////////////////////////
// strict-sorted render
void CDSGraphManager::r_dsgraph_render_sorted(bool render_hud)
{
	{
		PROF_EVENT("r_dsgraph_render_sorted");
		// Rendering
		r_dsgraph_render_graph_sorted(RGraph.mapStaticSorted.Sorted, true);
		r_dsgraph_render_graph_sorted(RGraph.mapDynamicSorted.Sorted, true);
	}

	if (render_hud)
		r_dsgraph_render_sorted_hud();

	// Camera Script Attachments
	if (RGraph.mapCamAttachedSorted.Sorted.size())
	{
		RImplementation.rmNear();
		// Change projection
		CHudInitializer initializer(2);

		// Rendering
		r_dsgraph_render_graph_sorted(RGraph.mapCamAttachedSorted.Sorted, true);
		RImplementation.rmNormal();
	}
}

void CDSGraphManager::r_dsgraph_capture_hud()
{
	if (g_hud)
	{
		g_hud->Render_Last(dcast_IPortalTraverser());
		set_Object();
	}
}

<<<<<<< HEAD
#if defined(USE_DX11)
//////////////////////////////////////////////////////////////////////////
// strict-sorted render
void CDSGraphManager::r_dsgraph_render_ScopeSorted()  //  Redotix99: for 3D Shader Based Scopes 	
{
	// Change projection
	CHudInitializer initializer(true);

	// Rendering
	RImplementation.rmNear();
	r_dsgraph_render_graph_sorted(RGraph.mapScopeHUDSorted, true);
	RImplementation.rmNormal();
}
#endif

void CDSGraphManager::r_dsgraph_render_sorted_hud()
{
	PROF_EVENT("r_dsgraph_render_sorted_hud");
#if	RENDER==R_R4
	HW.pContext->CopyResource(RImplementation.Target->rt_Accumulator->pSurface, RImplementation.Target->rt_Generic_0->pSurface);
#endif
	CHudInitializer initializer(true);

	RImplementation.rmNear();
	r_dsgraph_render_graph_sorted(RGraph.mapHUDSorted.Sorted, true);
	RImplementation.rmNormal();
}

=======
>>>>>>> april26
//////////////////////////////////////////////////////////////////////////
// strict-sorted render
void CDSGraphManager::r_dsgraph_render_emissive(bool clear, bool renderHUD)
{
	PROF_EVENT("r_dsgraph_render_emissive");
#if	RENDER!=R_R1
<<<<<<< HEAD
	r_dsgraph_render_graph_sorted(RGraph.mapStaticSorted.Emissive, clear);
	r_dsgraph_render_graph_sorted(RGraph.mapDynamicSorted.Emissive, clear);
=======
	// Sorted (back to front)
	mapEmissive.traverseLR(sorted_L1);

	// Change projection
	Fmatrix FTold = Device.mFullTransform;
>>>>>>> april26
	
	//	HACK: Calculate this only once
	CHudInitializer initializer(true);

	RImplementation.rmNear();
	r_dsgraph_render_graph_sorted(RGraph.mapHUDSorted.Emissive, clear);
	
	if (renderHUD)
		r_dsgraph_render_graph_sorted(RGraph.mapHUDSorted.Sorted, false);

	RImplementation.rmNormal();
#endif
}

void CDSGraphManager::r_dsgraph_render_water_ssr()
{
<<<<<<< HEAD
#ifdef USE_DX11
	PROF_EVENT("r_dsgraph_render_water_ssr");
	std::sort(RGraph.mapWater.begin(), RGraph.mapWater.end());
	for (auto& N : RGraph.mapWater)
	{
		dxRender_Visual* V = N.pVisual;
		VERIFY(V);

		RCache.set_Shader(RImplementation.Target->s_ssfx_water_ssr);

		RCache.set_xform_world(*N.pMatrix);
		RImplementation.apply_object(N.pObject);
		RImplementation.apply_lmaterial();

		RCache.set_c("cam_pos", RImplementation.Target->Position_previous.x, RImplementation.Target->Position_previous.y, RImplementation.Target->Position_previous.z, 0.0f);

		// Previous matrix data
		RCache.set_c("m_current", RImplementation.Target->Matrix_current);
		RCache.set_c("m_previous", RImplementation.Target->Matrix_previous);

		V->Render(calcLOD(N.ssa, V->vis.sphere.R));
	}
#endif
=======
	mapWater[Device.m_SecondViewport.IsSVPFrame()].traverseLR(water_node_ssr);
>>>>>>> april26
}

void CDSGraphManager::r_dsgraph_render_water()
{
<<<<<<< HEAD
	PROF_EVENT("r_dsgraph_render_water_ssr");
    std::sort(RGraph.mapWater.begin(), RGraph.mapWater.end());
    for (auto& N : RGraph.mapWater)
	{
		dxRender_Visual* V = N.pVisual;
		VERIFY(V);

#ifdef USE_DX11
		if (RImplementation.o.ssfx_water)
		{
			RCache.set_Shader(RImplementation.Target->s_ssfx_water);
		}
#endif

		RCache.set_xform_world(*N.pMatrix);
		RImplementation.apply_object(N.pObject);
		RImplementation.apply_lmaterial();

		// Wind settings
		float WindDir = g_pGamePersistent->Environment().CurrentEnv->wind_direction;
		float WindVel = g_pGamePersistent->Environment().CurrentEnv->wind_velocity;
		RCache.set_c("wind_setup", WindDir, WindVel, 0, 0);

		V->Render(calcLOD(N.ssa, V->vis.sphere.R));
	}
	RGraph.mapWater.clear();
=======
	mapWater[Device.m_SecondViewport.IsSVPFrame()].traverseLR(water_node);
	mapWater[Device.m_SecondViewport.IsSVPFrame()].clear();
>>>>>>> april26
}

//////////////////////////////////////////////////////////////////////////
// strict-sorted render
void CDSGraphManager::r_dsgraph_render_wmarks()
{
	PROF_EVENT("r_dsgraph_render_wmarks");
#if	RENDER!=R_R1
	// Rendering
	r_dsgraph_render_graph_sorted(RGraph.mapStaticSorted.Wmark);
	r_dsgraph_render_graph_sorted(RGraph.mapDynamicSorted.Wmark);
	//	HACK: Calculate this only once
	CHudInitializer initalizer(true);

	RImplementation.rmNear();
	r_dsgraph_render_graph_sorted(RGraph.mapHUDSorted.Wmark);
	RImplementation.rmNormal();
#endif
}

//////////////////////////////////////////////////////////////////////////
// strict-sorted render
void CDSGraphManager::r_dsgraph_render_distort()
{
	PROF_EVENT("r_dsgraph_render_distort");
	// Rendering
	r_dsgraph_render_graph_sorted(RGraph.mapStaticSorted.Distort, true);
	r_dsgraph_render_graph_sorted(RGraph.mapDynamicSorted.Distort, true);
	//	HACK: Calculate this only once
	CHudInitializer initalizer(true);

	RImplementation.rmNear();
	r_dsgraph_render_graph_sorted(RGraph.mapHUDSorted.Distort);
	RImplementation.rmNormal();
}

#include"LightTrack.h"
void CDSGraphManager::r_dsgraph_capture_static()
{
	PROF_EVENT("r_dsgraph_capture_static")
	const bool dbg_enabled = PortalTraverseDbg_Enabled();
	PortalTraverseDebugStats* dbg = dbg_enabled ? &PortalTraverseDbg_Get() : nullptr;
	const bool opt_bucket = PortalTraverseDbg_IsOptions(i_options);

	if (i_start)
	{
		// Traverse sector/portal structure
		if (psDeviceFlags.test(rsDrawStatic))
		{
			if (dbg)
			{
				const u32 sector_nodes = u32(m_sector_frustums.size());
				dbg->static_sector_nodes += sector_nodes;
				if (opt_bucket)
					dbg->static_sector_nodes_opt += sector_nodes;
				else
					dbg->static_sector_nodes_noopt += sector_nodes;
			}

			// Determine visibility for static geometry hierrarhy
			for (auto& pair : m_sector_frustums)
			{
				xr_vector<CFrustum>& frustums = pair.val.first;
				if (frustums.empty())
					continue;

				if (frustums.size() == 1)
				{
					if (dbg)
					{
						++dbg->static_frustum_nodes;
						++dbg->static_add_root_calls;
						if (opt_bucket)
						{
							++dbg->static_frustum_nodes_opt;
							++dbg->static_add_root_calls_opt;
						}
						else
						{
							++dbg->static_frustum_nodes_noopt;
							++dbg->static_add_root_calls_noopt;
						}
					}
					add_Static((IRenderVisual*)pair.key->root(), frustums.front(), frustums.front().getMask());
					continue;
				}

				xr_vector<u32> masks;
				masks.reserve(frustums.size());
				for (CFrustum& frustum_node : frustums)
					masks.push_back(frustum_node.getMask());

				if (dbg)
				{
					dbg->static_frustum_nodes += u32(frustums.size());
					++dbg->static_add_root_calls;
					if (opt_bucket)
					{
						dbg->static_frustum_nodes_opt += u32(frustums.size());
						++dbg->static_add_root_calls_opt;
					}
					else
					{
						dbg->static_frustum_nodes_noopt += u32(frustums.size());
						++dbg->static_add_root_calls_noopt;
					}
				}

				add_Static_MultiFrustum((IRenderVisual*)pair.key->root(), frustums, masks);
			}
		}
	}
}

void CDSGraphManager::r_dsgraph_capture_lights()
{
	PROF_EVENT("r_dsgraph_capture_lights")
	g_SpatialSpaceLights->q_frustum
	(
		lstLights,
		ISpatial_DB::O_ORDERED,
		STYPE_LIGHTSOURCE,
		i_frustum
	);

#if	RENDER==R_R1
	std::sort(lstLights.begin(), lstLights.end(), [](const ISpatialShared& _1, const ISpatialShared& _2) noexcept
	{
		if (!_1.get() || !_2.get()) return false;

		return	_1->spatial.sphere.P.distance_to_sqr(Device.vCameraPosition_saved) < _2->spatial.sphere.P.distance_to_sqr(Device.vCameraPosition_saved);
	});
#endif

	for (ISpatialShared spatial : lstLights)
	{
		if (0 == spatial) continue; spatial->spatial_updatesector();
		CSector* sector = (CSector*)spatial->spatial.sector;
		if (0 == sector) continue;

		if (!RImplementation.HOM.visible(spatial->spatial.sphere)) continue;

		if ((spatial->spatial.type & STYPE_LIGHTSOURCE))
		{
			// lightsource
			if (light* L = (light*)(spatial->dcast_Light()))
			{
#if	RENDER==R_R1
				RImplementation.L_DB->add_light(L);
#else
				if (L->get_LOD() > ps_r2_shadow_lod_min && L->has_light_visible_from_sectors(*this))
				{
					RImplementation.Lights.add_light(L);
				}
#endif
			}
		}
	}
}

void CDSGraphManager::r_dsgraph_capture_dynamic(CObject* O)
{
	PROF_EVENT("r_dsgraph_capture_dynamic")
	const bool dbg_enabled = PortalTraverseDbg_Enabled();
	PortalTraverseDebugStats* dbg = dbg_enabled ? &PortalTraverseDbg_Get() : nullptr;
	const bool opt_bucket = PortalTraverseDbg_IsOptions(i_options);

	if (i_start)
	{
		if (psDeviceFlags.test(rsDrawDynamic))
		{
			// Traverse object database
			g_SpatialSpace->q_frustum
			(
				lstRenderables,
				ISpatial_DB::O_ORDERED,
				i_doptions,
				i_frustum
			);
			if (dbg)
			{
				const u32 spatial_count = u32(lstRenderables.size());
				dbg->dynamic_spatials += spatial_count;
				if (opt_bucket)
					dbg->dynamic_spatials_opt += spatial_count;
				else
					dbg->dynamic_spatials_noopt += spatial_count;
			}
			set_Object();
#if	RENDER==R_R1
			if (i_mask[CDSGraphManager::fl_normal])//normal phase
			{
				std::sort(lstRenderables.begin(), lstRenderables.end(), [](const ISpatialShared& _1, const ISpatialShared& _2) noexcept
				{
					if (!_1.get() || !_2.get()) return false;

					return	_1->spatial.sphere.P.distance_to_sqr(Device.vCameraPosition_saved) < _2->spatial.sphere.P.distance_to_sqr(Device.vCameraPosition_saved);
				});

				if (ps_actor_shadow_flags.test(1))
					g_hud->Render_First(dcast_IPortalTraverser());

				r_dsgraph_capture_hud();
			}
#endif
			u32 uID_LTRACK = u32(-1);
			if (i_mask[CDSGraphManager::fl_normal])//normal phase
			{
				// update light-vis for current entity / actor
				if (CObject* O = g_pGameLevel->CurrentViewEntity())
				{
					if (!O->getDestroy())
					{
						if (CROS_impl* R = (CROS_impl*)O->ROS())
							R->update(O);
					}
				}

				RImplementation.uLastLTRACK++;
				if (!lstRenderables.empty())
				{
					uID_LTRACK = RImplementation.uLastLTRACK % lstRenderables.size();
#if	RENDER!=R_R1
					// update light-vis for selected entity
					// track lighting environment
					if (IRenderable* renderable = (IRenderable*)lstRenderables[uID_LTRACK]->dcast_Renderable())
					{
						if (CROS_impl* T = (CROS_impl*)renderable->renderable_ROS())
							T->update(renderable);
					}
#endif
				}

			}

			// Determine visibility for dynamic part of scene
			for (u32 o_it = 0; o_it < lstRenderables.size(); o_it++)
			{
				ISpatialShared spatial = lstRenderables[o_it];
				if (0 == spatial) continue;
				CSector* sector = (CSector*)spatial->spatial.sector;
				if (0 == sector) continue;

				if (i_mask[CDSGraphManager::fl_normal] && !RImplementation.HOM.visible(spatial->spatial.sphere))
					continue;

#if	RENDER==R_R1
				if ((spatial->spatial.type & STYPE_GLOW))
				{
					if (CGlow* glow = spatial->dcast_CGlow())
					{
						// It may be an glow
						RImplementation.L_Glows->add(glow);
					}
					continue;
				}
#endif

//				if ((spatial->spatial.type & STYPE_LIGHTSOURCE))
//				{
//					// lightsource
//					if (light* L = (light*)(spatial->dcast_Light()))
//					{
//#if	RENDER==R_R1
//						RImplementation.L_DB->add_light(L);
//#else
//						if (L->get_LOD() > EPS_L && L->has_light_visible_from_sectors(PT))
//						{
//							RImplementation.Lights.add_light(L);
//						}
//#endif
//					}
//					continue;
//				}

				if(!(spatial->spatial.type & STYPE_RENDERABLE) && !(spatial->spatial.type & STYPE_PARTICLE) && !(spatial->spatial.type & STYPE_RENDERABLESHADOW))
					continue;
				if (!is_sector_visible(sector))
					continue;

				for (CFrustum& frustum : m_sector_frustums.find(sector)->val.first)
				{
					if (dbg)
					{
						++dbg->dynamic_frustum_tests;
						if (opt_bucket)
							++dbg->dynamic_frustum_tests_opt;
						else
							++dbg->dynamic_frustum_tests_noopt;
					}

					if (frustum.testSphere_dirty(spatial->spatial.sphere.P, spatial->spatial.sphere.R))
					{
						if (dbg)
						{
							++dbg->dynamic_frustum_hits;
							if (opt_bucket)
								++dbg->dynamic_frustum_hits_opt;
							else
								++dbg->dynamic_frustum_hits_noopt;
						}

						// renderable
						IRenderable* renderable = spatial->dcast_Renderable();
						if (0 == renderable) break;

						if (O && O->dcast_Renderable() == renderable) break;

						// Rendering
#if	RENDER==R_R1
						if (i_mask[CDSGraphManager::fl_normal] && o_it == uID_LTRACK && renderable->renderable_ROS())
						{
							// track lighting environment
							if(CROS_impl* T = (CROS_impl*)renderable->renderable_ROS())
								T->update(renderable);
						}
#endif
						if (i_mask[CDSGraphManager::fl_normal] && !(spatial->spatial.type & STYPE_PARTICLE))
							set_Object(renderable);

						renderable->renderable_Render(dcast_IPortalTraverser());
						if (dbg)
						{
							++dbg->dynamic_rendered;
							if (opt_bucket)
								++dbg->dynamic_rendered_opt;
							else
								++dbg->dynamic_rendered_noopt;
						}

						if (i_mask[CDSGraphManager::fl_normal] && !(spatial->spatial.type & STYPE_PARTICLE))
							set_Object();
						break;
					}
				}
			}
		}
	}
}

void CDSGraphManager::r_dsgraph_capture(bool lights, bool dynamic, CObject* O)
{
	PROF_EVENT("r_dsgraph_capture")
	r_dsgraph_capture_static();

	if(lights)
		r_dsgraph_capture_lights();

	if (dynamic)
		r_dsgraph_capture_dynamic(O);
}
