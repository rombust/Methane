/*
**  ClanLib SDK
**  Copyright (c) 1997-2020 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
**    Mark Page
*/

#include "precomp.h"
#include "canvas_batcher.h"
#include "API/Display/Render/render_batcher.h"
#include "API/Display/Render/shared_gc_data.h"
#include "API/Display/TargetProviders/graphic_context_provider.h"

namespace clan
{
	class CanvasBatcher_Impl
	{
	public:
		CanvasBatcher_Impl(GraphicContext &gc);
		~CanvasBatcher_Impl();

		void flush();
		bool set_batcher(GraphicContext &gc, RenderBatcher *batcher);
		void stop_batcher(GraphicContext& gc);
		void update_batcher_matrix(GraphicContext &gc, const Mat4f &modelview, const Mat4f &projection, TextureImageYAxis image_yaxis);

		GraphicContext current_gc;

		RenderBatcher *active_batcher;
	};

	CanvasBatcher_Impl::CanvasBatcher_Impl(GraphicContext &gc) : active_batcher(nullptr)
	{

	}

	CanvasBatcher_Impl::~CanvasBatcher_Impl()
	{
	}

	CanvasBatcher::CanvasBatcher()
	{
	}

	CanvasBatcher::CanvasBatcher(GraphicContext &gc) : impl(std::make_shared<CanvasBatcher_Impl>(gc))
	{
	}

	CanvasBatcher::~CanvasBatcher()
	{
	}

	void CanvasBatcher_Impl::flush()
	{
		if (active_batcher)
		{
			RenderBatcher *batcher = active_batcher;
			active_batcher = nullptr;
			batcher->flush(current_gc);
		}
	}

	void CanvasBatcher_Impl::update_batcher_matrix(GraphicContext &gc, const Mat4f &modelview, const Mat4f &projection, TextureImageYAxis image_yaxis)
	{
		if (gc != current_gc)
		{
			flush();
			current_gc = gc;
		}

		if (active_batcher)
		{
			active_batcher->matrix_changed(modelview, projection, image_yaxis, gc.get_pixel_ratio());
		}
	}

	void CanvasBatcher_Impl::stop_batcher(GraphicContext& gc)
	{
		if (gc == current_gc)
		{
			flush();
			current_gc = clan::GraphicContext();
			active_batcher = nullptr;
		}
	}

	bool CanvasBatcher_Impl::set_batcher(GraphicContext &gc, RenderBatcher *batcher)
	{
		if ((active_batcher != batcher) || (gc != current_gc))
		{
			flush();
			current_gc = gc;
			active_batcher = batcher;
			return true;
		}
		return false;
	}

	void CanvasBatcher::flush()
	{
		impl->flush();
	}

	void CanvasBatcher::update_batcher_matrix(GraphicContext &gc, const Mat4f &modelview, const Mat4f &projection, TextureImageYAxis image_yaxis)
	{
		impl->update_batcher_matrix(gc, modelview, projection, image_yaxis);
	}

	bool CanvasBatcher::set_batcher(GraphicContext &gc, RenderBatcher *batcher)
	{
		return impl->set_batcher(gc, batcher);
	}

	void CanvasBatcher::stop_batcher(GraphicContext& gc)
	{
		impl->stop_batcher(gc);
	}

}
