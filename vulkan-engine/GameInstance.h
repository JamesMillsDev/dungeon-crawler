#pragma once


/** @brief Callback type invoked when an Actor is fully destroyed. */


/**
 * @brief Abstract base class for the game logic layer.
 *
 * GameInstance defines the interface that Application calls during the
 * main loop. Derive from this class and implement all four virtual methods
 * to provide game-specific behaviour. The derived type is passed as a
 * template argument to Application::Open(), which constructs and owns
 * the instance for the duration of the session.
 */
class GameInstance
{
public:
	/** @brief Constructs the GameInstance and creates the root Actor. */
	GameInstance() = default;

	/** @brief Virtual destructor to ensure correct cleanup of derived types. */
	virtual ~GameInstance() = default;

public:
	/**
	 * @brief Spawns a new Actor in the scene, parented to the root Actor.
	 *
	 * The GameInstance retains shared ownership of the Actor for the
	 * duration of its lifetime.
	 *
	 * @return A shared_ptr to the newly created Actor.
	 */


	 /**
	  * @brief Queues an Actor for destruction at the end of the current frame.
	  *
	  * The Actor is not destroyed immediately; it is removed from the scene
	  * once the current Tick/Render cycle has completed, to avoid invalidating
	  * iterators mid-frame.
	  *
	  * @param actor A non-owning pointer identifying the Actor to destroy.
	  */


	  /**
	   * @brief Called once before the main loop begins.
	   *
	   * Use this to load assets, set up scene state, register systems,
	   * or perform any other one-time startup work.
	   */
	virtual void Init() = 0;

	/**
	 * @brief Called once after the main loop exits.
	 *
	 * Use this to release resources, save state, or perform any other
	 * cleanup that must happen before the application terminates.
	 */
	virtual void Shutdown() = 0;

	/**
	 * @brief Called once per frame to advance game logic.
	 *
	 * All simulation, input handling, and non-rendering updates should
	 * be performed here. Render() is called immediately after.
	 *
	 * @param dt The time elapsed since the last frame, in seconds.
	 */
	virtual void Tick(float dt) = 0;

	/**
	 * @brief Called once per frame to submit draw commands.
	 *
	 * Invoked by Application between BeginFrame() and EndFrame(), so
	 * the raylib drawing context is already active when this is called.
	 * All rendering should be performed here rather than in Tick().
	 */
	virtual void Render() = 0;

private:
	/** @brief The root of the Actor hierarchy; all spawned Actors are parented to this. */


	/** @brief All Actors currently alive in the scene, owned by this GameInstance. */


	/** @brief Destruction callbacks queued during the frame, flushed at frame end. */


private:
	/**
	 * @brief Calls Tick() on all Actors owned by this GameInstance.
	 * @param dt Elapsed time in seconds since the last frame.
	 */


	 /**
	  * @brief Calls Render() on all Actors owned by this GameInstance.
	  *
	  * Should be called after TickActors() each frame, within an active
	  * BeginFrame()/EndFrame() block.
	  */


};