#include "coroutine.hpp"
#include "threadlocal.hpp"

#include <sys/mman.h>

sprawl::threading::Coroutine::Holder::Holder()
	: m_function(nullptr)
	, m_stackSize(0)
	, m_stack(nullptr)
	, m_stackPointer(nullptr)
	, m_state(CoroutineState::Created)
	, m_context()
	, m_refCount(1)
	, m_priorCoroutine(nullptr)
{
	m_stackPointer = &m_context;

	getcontext(&m_context);
}

sprawl::threading::Coroutine::Holder::Holder(std::function<void()> function, size_t stackSize)
	: m_function(function)
	, m_stackSize(stackSize == 0 ? 1024 * 1024 : stackSize)
	, m_stack(nullptr)
	, m_stackPointer(nullptr)
	, m_state(CoroutineState::Created)
	, m_context()
	, m_refCount(1)
	, m_priorCoroutine(nullptr)
{
	m_stack = mmap(NULL, m_stackSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

	m_stackPointer = &m_context;

	getcontext(&m_context);

	m_context.uc_link = nullptr;
	m_context.uc_stack.ss_sp = m_stack;
	m_context.uc_stack.ss_size = m_stackSize;

	makecontext(&m_context, &Coroutine::entryPoint_, 0);
}

void sprawl::threading::Coroutine::Resume()
{
	m_holder->m_state = CoroutineState::Executing;

	m_holder->m_priorCoroutine = *ms_thisThreadCoroutine;

	ms_thisThreadCoroutine = *this;

	m_holder->m_priorCoroutine.m_holder->m_state = CoroutineState::Paused;

	ms_coroutineInitHelper = this;
	swapcontext(&m_holder->m_priorCoroutine.m_holder->m_context, &m_holder->m_context);
}

void sprawl::threading::Coroutine::reactivate_()
{
	m_holder->m_state = CoroutineState::Executing;

	Coroutine currentlyActiveCoroutine = *ms_thisThreadCoroutine;
	ms_thisThreadCoroutine = *this;

	swapcontext(&currentlyActiveCoroutine.m_holder->m_context, &m_holder->m_context);
}

void sprawl::threading::Coroutine::Pause()
{
	m_holder->m_state = CoroutineState::Paused;

	ms_thisThreadCoroutine = m_holder->m_priorCoroutine;

	m_holder->m_priorCoroutine.m_holder->m_state = CoroutineState::Executing;

	swapcontext(&m_holder->m_context, &m_holder->m_priorCoroutine.m_holder->m_context);
}
